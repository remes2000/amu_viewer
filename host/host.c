#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <jpeglib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define DEFAULT_TCP_PORT 1337
#define DEFAULT_UDP_PORT DEFAULT_TCP_PORT + 1
#define DEFAULT_TCP_FILE_TRANSFER_PORT DEFAULT_TCP_PORT + 2
#define MAX_UDP_PACKET_SIZE 65535
#define MAX_IMAGE_SIZE_WHEN_ADJUST 60535
#define LOG(format, ...) printf("[%s] " format "\n", getFormattedTime(), ## __VA_ARGS__)
#define JPEG_START_QUALITY 20
#define JPEG_DOWN_QUALITY 5

// ---=== EVENT CODES ===---
#define EVENT_MOUSE_MOVE 0
#define EVENT_MOUSE_LEFT_PRESSED 1
#define EVENT_MOUSE_LEFT_RELEASED 2
#define EVENT_MOUSE_RIGHT_PRESSED 3
#define EVENT_MOUSE_RIGHT_RELEASED 4
#define EVENT_KEY_PRESSED 5
#define EVENT_KEY_RELEASED 6
#define EVENT_CLOSE_CONNECTION 7
// ---=== EVENT CODES ===---

char* getFormattedTime(void) {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    static char _retval[20];
    strftime(_retval, sizeof(_retval), "%Y-%m-%d %H:%M:%S", timeinfo);
    return _retval;
}

struct x_connection {
    Display * disp;
    Window root;
};

struct frame_settings {
    int jpeg_quality;
    int scale;
};

struct amu_viewer_setup {
    unsigned int access_code;
    int host_socket_tcp;
    int host_socket_tcp_file_transfer;
    int host_socket_udp;
    int client_socket_tcp;
    struct sockaddr_in tcp_host_address;
    struct sockaddr_in tcp_file_transfer_host_address;
    struct sockaddr_in udp_host_address;
    struct sockaddr_in tcp_client_address;
    struct sockaddr_in udp_client_address;
    struct x_connection event_x_connection;
    struct x_connection screen_broadcast_x_connection;
    int is_broadcast_possible;
    struct frame_settings frame_settings;
    pid_t screen_broadcast_process;
    pid_t file_transfer_process;
};

struct screen_image {
    XImage * image;
    XWindowAttributes window_attributes;
};

struct frame {
  unsigned char * image;
  unsigned long image_size;
};

socklen_t socklen = sizeof(struct sockaddr_in);
int setup(int argc, char **argv, struct amu_viewer_setup * viewer_setup);
int setup_host_address(int argc, char **argv, struct amu_viewer_setup * viewer_setup);
void setup_connections_to_x_server(struct amu_viewer_setup * viewer_setup);
void setup_adjust_frame_settings(struct amu_viewer_setup * viewer_setup);
void get_screen_image(struct x_connection * connection, struct screen_image * raw_screen_image);
void setup_access_code(struct amu_viewer_setup * viewer_setup);
int setup_host_sockets(struct amu_viewer_setup * viewer_setup);
int reset(struct amu_viewer_setup * viewer_setup);
void say_hello(struct amu_viewer_setup * viewer_setup);
int handle_connection(struct amu_viewer_setup * viewer_setup);
int authorize(struct amu_viewer_setup * viewer_setup, int tcp_socket);
int set_udp_client_address(struct amu_viewer_setup * viewer_setup);
int read_unsigned_int(int tcp_socket, unsigned int * dest);
int read_unsigned_short(int tcp_socket, unsigned short * dest);
int read_n_bytes(int tcp_socket, unsigned char * dest, unsigned int n);
int download_file(int tcp_socket, unsigned char * filename, unsigned int file_size);
int begin_screen_broadcast(struct amu_viewer_setup * viewer_setup);
int begin_file_transfer_process(struct amu_viewer_setup * viewer_setup);
int handle_file_transfer_connection(struct amu_viewer_setup * viewer_setup, int client_file_transfer_socket);
void send_frame(struct amu_viewer_setup * viewer_setup);
void send_transmission_over_udp_not_possible_message(struct amu_viewer_setup * viewer_setup);
int handle_event(unsigned short event_code, struct amu_viewer_setup * viewer_setup);
int handle_mouse_move(struct amu_viewer_setup * viewer_setup);
int handle_mouse_left_pressed(struct amu_viewer_setup * viewer_setup);
int handle_mouse_left_released(struct amu_viewer_setup * viewer_setup);
int handle_mouse_right_pressed(struct amu_viewer_setup * viewer_setup);
int handle_mouse_right_released(struct amu_viewer_setup * viewer_setup);
int handle_key_pressed(struct amu_viewer_setup * viewer_setup);
int handle_key_released(struct amu_viewer_setup * viewer_setup);
void get_frame(struct frame * screen_frame, struct screen_image * screen_image, int quality, int scale);

int main(int argc, char **argv) {
    srand(time(NULL));
    struct amu_viewer_setup viewer_setup;
    if(setup(argc, argv, &viewer_setup) == -1) {
        printf("setup() failed \n");
        return -1;
    }
    say_hello(&viewer_setup);
    if(begin_file_transfer_process(&viewer_setup) == -1) {
        LOG("File transfer service init failed, file transfer not possible");
    }
    LOG("Waiting for connection...");
    while((viewer_setup.client_socket_tcp = accept(viewer_setup.host_socket_tcp, (struct sockaddr*) &viewer_setup.tcp_client_address, &socklen)) != -1) {
        if(handle_connection(&viewer_setup) == -1) {
            LOG("handle_connection() error");
        }
        reset(&viewer_setup);
        LOG("Waiting for connection...");
    }
    printf("accept() failed");
    close(viewer_setup.client_socket_tcp);
    close(viewer_setup.host_socket_tcp);
    close(viewer_setup.host_socket_tcp_file_transfer);
    close(viewer_setup.host_socket_udp);
    return 0;
}

int setup(int argc, char **argv, struct amu_viewer_setup * viewer_setup) {
    if(setup_host_address(argc, argv, viewer_setup) == -1) {
        return -1;
    }
    if(setup_host_sockets(viewer_setup) == -1) {
        return -1;
    }
    setup_connections_to_x_server(viewer_setup);
    setup_adjust_frame_settings(viewer_setup);
    setup_access_code(viewer_setup);
    return 0;
}

int setup_host_address(int argc, char **argv, struct amu_viewer_setup * viewer_setup) {
    unsigned short tcp_port = DEFAULT_TCP_PORT;
    unsigned short udp_port = DEFAULT_UDP_PORT;
    unsigned short tcp_file_transfer_port = DEFAULT_TCP_FILE_TRANSFER_PORT;

    if(argc > 1) {
        tcp_port = atoi(argv[1]);
    }
    if(argc > 2) {
        udp_port = atoi(argv[2]);
    }
    if(argc > 3) {
        tcp_file_transfer_port = atoi(argv[3]);
    }
    viewer_setup->tcp_host_address.sin_family = AF_INET;
    viewer_setup->tcp_host_address.sin_port = htons(tcp_port);
    viewer_setup->tcp_host_address.sin_addr.s_addr = INADDR_ANY;
    viewer_setup->udp_host_address.sin_family = AF_INET;
    viewer_setup->udp_host_address.sin_port = htons(udp_port);
    viewer_setup->udp_host_address.sin_addr.s_addr = INADDR_ANY;
    viewer_setup->tcp_file_transfer_host_address.sin_family = AF_INET;
    viewer_setup->tcp_file_transfer_host_address.sin_port = htons(tcp_file_transfer_port);
    viewer_setup->tcp_file_transfer_host_address.sin_addr.s_addr = INADDR_ANY;
    return 0;
}

int setup_host_sockets(struct amu_viewer_setup * viewer_setup) {
    // host tcp socket
    viewer_setup->host_socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if(viewer_setup->host_socket_tcp == -1) {
        printf("setup_host_sockets() failed, on tcp socket() \n");
        return -1;
    }
    if(bind(viewer_setup->host_socket_tcp, (struct sockaddr*) &viewer_setup->tcp_host_address, sizeof(viewer_setup->tcp_host_address)) == -1) {
        printf("setup_host_sockets() failed, on tcp bind() \n");
        return -1;
    }
    if (listen(viewer_setup->host_socket_tcp, 1) == -1) {
        printf("setup_host_sockets() failed, on listen() \n");
        return -1; 
    }
    // host tcp file transfer socket
    viewer_setup->host_socket_tcp_file_transfer = socket(AF_INET, SOCK_STREAM, 0);
    if(viewer_setup->host_socket_tcp_file_transfer == -1) {
        printf("setup_host_sockets() failed, on tcp file transfer socket() \n");
        return -1;
    }
    if(bind(viewer_setup->host_socket_tcp_file_transfer, (struct sockaddr*) &viewer_setup->tcp_file_transfer_host_address, sizeof(viewer_setup->tcp_file_transfer_host_address)) == -1) {
        printf("setup_host_sockets() failed, on tcp file transfer bind() \n");
        return -1;
    }
    if (listen(viewer_setup->host_socket_tcp_file_transfer, 1) == -1) {
        printf("setup_host_sockets() failed, on listen() tcp file transfer \n");
        return -1;
    }
    // host udp socket
    viewer_setup->host_socket_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if(viewer_setup->host_socket_udp == -1) {
        printf("setup_host_sockets() failed, on udp socket() \n");
        return -1;
    }
    if(bind(viewer_setup->host_socket_udp, (struct sockaddr*) &viewer_setup->udp_host_address, sizeof(viewer_setup->udp_host_address)) == -1) {
        printf("setup_host_sockets() failed, on udp bind() \n");
        return -1;
    }
    return 0;
}

void setup_connections_to_x_server(struct amu_viewer_setup * viewer_setup) {
    viewer_setup->event_x_connection.disp = XOpenDisplay(0);
    viewer_setup->event_x_connection.root = XRootWindow(viewer_setup->event_x_connection.disp, 0);
    XSelectInput(viewer_setup->event_x_connection.disp, viewer_setup->event_x_connection.root, KeyReleaseMask);

    viewer_setup->screen_broadcast_x_connection.disp = XOpenDisplay(0);
    viewer_setup->screen_broadcast_x_connection.root = XRootWindow(viewer_setup->screen_broadcast_x_connection.disp, 0);
    XSelectInput(viewer_setup->screen_broadcast_x_connection.disp, viewer_setup->screen_broadcast_x_connection.root, KeyReleaseMask);
}

void setup_access_code(struct amu_viewer_setup * viewer_setup) {
    viewer_setup->access_code = (rand() % 901) + 100;
}

void setup_adjust_frame_settings(struct amu_viewer_setup * viewer_setup) {
    int scale_possibilities[] = {1, 2, 4, 8, 16};
    struct screen_image raw_screen_image;
    struct frame frame;
    get_screen_image(&viewer_setup->event_x_connection, &raw_screen_image);
    LOG("Adjusting frame settings:");
    for(int i=0; i<5; i++) {
        int scale = scale_possibilities[i];
        int quality = JPEG_START_QUALITY;
        while(quality >= JPEG_DOWN_QUALITY) {
            get_frame(&frame, &raw_screen_image, quality, scale);
            LOG("scale = %d quality = %d [%d]", scale, quality, frame.image_size);
            free(frame.image);
            if(frame.image_size <= MAX_IMAGE_SIZE_WHEN_ADJUST) {
                viewer_setup->frame_settings.jpeg_quality = quality;
                viewer_setup->frame_settings.scale = scale;
                viewer_setup->is_broadcast_possible = 1;
                return;
            }
            quality--;
        }
    }
    viewer_setup->is_broadcast_possible = 0;
}

void get_screen_image(struct x_connection * connection, struct screen_image * raw_screen_image) {
    XWindowAttributes gwa;
    XGetWindowAttributes(connection->disp, connection->root, &gwa);
    raw_screen_image->image = XGetImage(connection->disp, connection->root, 0, 0, gwa.width, gwa.height, AllPlanes, ZPixmap);
    raw_screen_image->window_attributes = gwa;
}

int reset(struct amu_viewer_setup * viewer_setup) {
    close(viewer_setup->client_socket_tcp);
    kill(viewer_setup->screen_broadcast_process, SIGKILL);
    setup_connections_to_x_server(viewer_setup);
}

void say_hello(struct amu_viewer_setup * viewer_setup) {
    printf("---=== AMU_VIEWER ===---- \n");
    printf("Listening on port %d \n", ntohs(viewer_setup->tcp_host_address.sin_port));
    printf("UDP port: %d \n", ntohs(viewer_setup->udp_host_address.sin_port));
    printf("File transfer port: %d \n", ntohs(viewer_setup->tcp_file_transfer_host_address.sin_port));
    printf("Access code = %d\n", viewer_setup->access_code);
    if(viewer_setup->is_broadcast_possible) {
        printf("Frames will be sending in jpeg files, scale = %d quality = %d \n", viewer_setup->frame_settings.scale, viewer_setup->frame_settings.jpeg_quality);
    } else {
        printf("Frames transmission via udp is not possible because of too big screen size \n");
    }
}

int handle_connection(struct amu_viewer_setup * viewer_setup) {
    int authorization_result;

    LOG("Client %s connected", inet_ntoa(viewer_setup->tcp_client_address.sin_addr));
    authorization_result = authorize(viewer_setup, viewer_setup->client_socket_tcp);
    if(authorization_result == -1) {
        LOG("Authorization error");
        return -1;
    }
    if(authorization_result == 1) {
        if(send(viewer_setup->client_socket_tcp, &viewer_setup->udp_host_address.sin_port, sizeof(viewer_setup->udp_host_address.sin_port), 0) == -1) {
            LOG("An error occured while sending udp port to client");
            return -1;
        }
        if(send(viewer_setup->client_socket_tcp, &viewer_setup->tcp_file_transfer_host_address.sin_port, sizeof(viewer_setup->tcp_file_transfer_host_address.sin_port), 0) == -1) {
            LOG("An error occured while sending tcp file transfer port to client");
            return -1;
        }
        if(set_udp_client_address(viewer_setup) == -1) {
            LOG("An error occured while reading client udp port");
            return -1;
        }
        if(viewer_setup->is_broadcast_possible) {
            if(begin_screen_broadcast(viewer_setup) == -1) {
                LOG("An error occured while starting screen broadcast");
                return -1;
            }
        } else {
            send_transmission_over_udp_not_possible_message(viewer_setup);
        }
        int handle_event_result;
        unsigned short event;
        while(1) {
            if(read_unsigned_short(viewer_setup->client_socket_tcp, &event) == -1) {
                return -1;
            }
            handle_event_result = handle_event(event, viewer_setup);
            if(handle_event_result == -1 || handle_event_result == -2) {
                return handle_event_result;
            }
        }
    }
}

int authorize(struct amu_viewer_setup * viewer_setup, int tcp_socket) {
    uint8_t authorization_result;
    unsigned int provided_access_code;

    LOG("Waiting for access code...");
    if(read_unsigned_int(tcp_socket, &provided_access_code) == -1) {
        return -1;
    }
    LOG("Provided access code: %u", provided_access_code);
    if(provided_access_code == viewer_setup->access_code) {
        LOG("Access granted");
        authorization_result = 1;
    } else {
        LOG("Permission denied");
        authorization_result = 0;
    }
    if(send(tcp_socket, &authorization_result, sizeof(authorization_result), 0) != sizeof(authorization_result)) {
        return -1;
    }
    return authorization_result;
}

int set_udp_client_address(struct amu_viewer_setup * viewer_setup) {
    unsigned short client_udp_port;

    LOG("Waiting for client udp port...");
    if(read_unsigned_short(viewer_setup->client_socket_tcp, &client_udp_port) == -1) {
        return -1;
    }
    viewer_setup->udp_client_address.sin_family = AF_INET;
    viewer_setup->udp_client_address.sin_addr = viewer_setup->tcp_client_address.sin_addr;
    viewer_setup->udp_client_address.sin_port = htons(client_udp_port);
    LOG("Client udp address: %s:%d", inet_ntoa(viewer_setup->udp_client_address.sin_addr), client_udp_port);
    return 0;
}

int read_unsigned_int(int tcp_socket, unsigned int * dest) {
    char buffer[sizeof(unsigned int)];
    if(read_n_bytes(tcp_socket, buffer, sizeof(unsigned int)) == -1) {
        return -1;
    }
    *dest = ntohl(*((unsigned int *) buffer));
    return 0;
}

int read_unsigned_short(int tcp_socket, unsigned short * dest) {
    char buffer[sizeof(unsigned short)];
    if(read_n_bytes(tcp_socket, buffer, sizeof(unsigned short)) == -1) {
        return -1;
    }
    *dest = ntohs(*((unsigned short *) buffer));
    return 0;
}

int read_n_bytes(int tcp_socket, unsigned char * dest, unsigned int n) {
    int total_readed_bytes = 0;
    int readed_bytes = 0;
    while(total_readed_bytes < n) {
        readed_bytes = recv(tcp_socket, dest + total_readed_bytes, n - total_readed_bytes, 0);
        if(readed_bytes == -1) {
            return -1;
        }
        total_readed_bytes += readed_bytes;
    }
    return 0;
}

int download_file(int tcp_socket, unsigned char * filename, unsigned int file_size) {
    int total_readed_bytes = 0;
    int readed_bytes = 0;
    char buf[512];
    FILE *file;

    if((file = fopen(filename, "wb")) == NULL) {
        LOG("Cannot open file %s", filename);
        return -1;
    }

    while(total_readed_bytes < file_size) {
        readed_bytes = recv(tcp_socket, buf, 512, 0);
        if(readed_bytes == -1) {
            fclose(file);
            return -1;
        }
        fwrite(buf, readed_bytes, 1, file);
        total_readed_bytes += readed_bytes;
    }
    fclose(file);
}

int begin_file_transfer_process(struct amu_viewer_setup * viewer_setup) {
    LOG("File transfer service, creating child process...");
    pid_t pid;
    if((pid = fork()) == -1) {
        LOG("fork() failed");
        return -1;
    }
    if(pid == 0) {
        int file_transfer_client_socket_tcp;
        while((file_transfer_client_socket_tcp = accept(viewer_setup->host_socket_tcp_file_transfer, (struct sockaddr*) &viewer_setup->tcp_file_transfer_host_address, &socklen)) != -1) {
            LOG("File transfer connection begin");
            if(handle_file_transfer_connection(viewer_setup, file_transfer_client_socket_tcp) == -1) {
                LOG("handle_file_transfer_connection() error");
            }
            LOG("File transfer connection end");
            close(file_transfer_client_socket_tcp);
        }
        LOG("file transfer accept() failed");
        close(viewer_setup->host_socket_tcp_file_transfer);
        exit(0);
    }
    viewer_setup->file_transfer_process = pid;
    LOG("File transfer process pid = %d", pid);
    return 0;
}

int handle_file_transfer_connection(struct amu_viewer_setup * viewer_setup, int client_file_transfer_socket) {
    int authorization_result;
    unsigned int file_size = 0, filename_size = 0;
    char * filename;

    authorization_result = authorize(viewer_setup, client_file_transfer_socket);
    if(authorization_result == -1) {
        LOG("Authorization error");
        return -1;
    }
    if(authorization_result == 1) {
        if(read_unsigned_int(client_file_transfer_socket, &filename_size) == -1) {
            LOG("Error while reading filename size");
            return -1;
        }
        filename = (char *) malloc((filename_size+1) * sizeof(char));
        if(read_n_bytes(client_file_transfer_socket, filename, filename_size) == -1) {
            LOG("Error while reading filename");
            free(filename);
            return -1;
        }
        filename[filename_size] = '\0';
        LOG("Downloading file %s...", filename);
        if(read_unsigned_int(client_file_transfer_socket, &file_size) == -1) {
            LOG("Error while reading file size");
            free(filename);
            return -1;
        }
        if(download_file(client_file_transfer_socket, filename, file_size) == -1) {
            LOG("Error while downloading file");
            free(filename);
            return -1;
        }
        LOG("File %s downloaded", filename);
        free(filename);
    }
}

int begin_screen_broadcast(struct amu_viewer_setup * viewer_setup) {
    LOG("Begin screen broadcast, creating child process...");
    pid_t pid;
    if((pid = fork()) == -1) {
        LOG("fork() failed");
        return -1;
    }
    if(pid == 0) {
        while(1) {
            send_frame(viewer_setup);
            usleep(300 * 1000);
        }
        exit(0);
    }
    viewer_setup->screen_broadcast_process = pid;
    LOG("Screen broadcast process pid = %d", pid);
    return 0;
}

void send_frame(struct amu_viewer_setup * viewer_setup) {
    struct frame screen_frame;
    struct screen_image image;
    get_screen_image(&viewer_setup->screen_broadcast_x_connection, &image);
    get_frame(&screen_frame, &image, viewer_setup->frame_settings.jpeg_quality, viewer_setup->frame_settings.scale);
    if(screen_frame.image_size <= MAX_UDP_PACKET_SIZE) {
        sendto(viewer_setup->host_socket_udp,
               screen_frame.image,
               screen_frame.image_size,
               0,
               (struct sockaddr*) &viewer_setup->udp_client_address,
               sizeof(viewer_setup->udp_client_address));
    } else {
        LOG("Frame could not be sent because of too big frame size (%d). Downgrading jpeg quality from %d to %d", screen_frame.image_size);
    }
    free(screen_frame.image);
}

void send_transmission_over_udp_not_possible_message(struct amu_viewer_setup * viewer_setup) {
    FILE * image_file;
    char * buffer;
    long file_size;

    if((image_file = fopen("./resources/transmission_not_possible.jpg", "r")) == NULL) {
        return;
    }
    fseek(image_file, 0L, SEEK_END);
    file_size = ftell(image_file);
    fseek(image_file, 0L, SEEK_SET);
    buffer = (char *)calloc(file_size, sizeof(char));
    fread(buffer, sizeof(char), file_size, image_file);
    fclose(image_file);

    sendto(viewer_setup->host_socket_udp,
           buffer,
           file_size,
           0,
           (struct sockaddr*) &viewer_setup->udp_client_address,
           sizeof(viewer_setup->udp_client_address));
    free(buffer);
}

int handle_event(unsigned short event_code, struct amu_viewer_setup * viewer_setup) {
    int result = 0;
    switch (event_code) {
        case EVENT_MOUSE_MOVE:
            result = handle_mouse_move(viewer_setup);
        break;
        case EVENT_MOUSE_LEFT_PRESSED:
            result = handle_mouse_left_pressed(viewer_setup);
        break;
        case EVENT_MOUSE_LEFT_RELEASED:
            result = handle_mouse_left_released(viewer_setup);
        break;
        case EVENT_MOUSE_RIGHT_PRESSED:
            result = handle_mouse_right_pressed(viewer_setup);
        break;
        case EVENT_MOUSE_RIGHT_RELEASED:
            result = handle_mouse_right_released(viewer_setup);
        break;
        case EVENT_KEY_PRESSED:
            result = handle_key_pressed(viewer_setup);
        break;
        case EVENT_KEY_RELEASED:
            result = handle_key_released(viewer_setup);
        break;
        case EVENT_CLOSE_CONNECTION:
            result = -2;
        break;
    }
    return result;
}

int handle_mouse_move(struct amu_viewer_setup * viewer_setup) {
    unsigned int x, y;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &x) == -1) {
        return -1;
    }
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &y) == -1) {
        return -1;
    }
    int scale = viewer_setup->frame_settings.scale;
    XWarpPointer(viewer_setup->event_x_connection.disp, None, viewer_setup->event_x_connection.root, 0, 0, 0, 0, x * scale, y * scale);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

int handle_mouse_left_pressed(struct amu_viewer_setup * viewer_setup) {
    unsigned int x, y;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &x) == -1) {
        return -1;
    }
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &y) == -1) {
        return -1;
    }
    int scale = viewer_setup->frame_settings.scale;
    XWarpPointer(viewer_setup->event_x_connection.disp, None, viewer_setup->event_x_connection.root, 0, 0, 0, 0, x * scale, y * scale);
    XTestFakeButtonEvent(viewer_setup->event_x_connection.disp, 1, True, CurrentTime);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

int handle_mouse_left_released(struct amu_viewer_setup * viewer_setup) {
    unsigned int x, y;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &x) == -1) {
        return -1;
    }
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &y) == -1) {
        return -1;
    }
    int scale = viewer_setup->frame_settings.scale;
    XWarpPointer(viewer_setup->event_x_connection.disp, None, viewer_setup->event_x_connection.root, 0, 0, 0, 0, x * scale, y * scale);
    XTestFakeButtonEvent(viewer_setup->event_x_connection.disp, 1, False, CurrentTime);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

int handle_mouse_right_pressed(struct amu_viewer_setup * viewer_setup) {
    unsigned int x, y;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &x) == -1) {
        return -1;
    }
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &y) == -1) {
        return -1;
    }
    int scale = viewer_setup->frame_settings.scale;
    XWarpPointer(viewer_setup->event_x_connection.disp, None, viewer_setup->event_x_connection.root, 0, 0, 0, 0, x * scale, y * scale);
    XTestFakeButtonEvent(viewer_setup->event_x_connection.disp, 3, True, CurrentTime);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

int handle_mouse_right_released(struct amu_viewer_setup * viewer_setup) {
    unsigned int x, y;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &x) == -1) {
        return -1;
    }
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &y) == -1) {
        return -1;
    }
    int scale = viewer_setup->frame_settings.scale;
    XWarpPointer(viewer_setup->event_x_connection.disp, None, viewer_setup->event_x_connection.root, 0, 0, 0, 0, x * scale, y * scale);
    XTestFakeButtonEvent(viewer_setup->event_x_connection.disp, 3, False, CurrentTime);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

int handle_key_pressed(struct amu_viewer_setup * viewer_setup) {
    unsigned int key_code;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &key_code) == -1) {
        return -1;
    }
    XTestFakeKeyEvent(viewer_setup->event_x_connection.disp, key_code, True, 0);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

int handle_key_released(struct amu_viewer_setup * viewer_setup) {
    unsigned int key_code;
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &key_code) == -1) {
        return -1;
    }
    XTestFakeKeyEvent(viewer_setup->event_x_connection.disp, key_code, False, 0);
    XFlush(viewer_setup->event_x_connection.disp);
    return 0;
}

void get_frame(struct frame * screen_frame, struct screen_image * screen_image, int quality, int scale) {
    int width = screen_image->window_attributes.width;
    int height = screen_image->window_attributes.height;

    unsigned long red_mask = screen_image->image->red_mask;
    unsigned long green_mask = screen_image->image->green_mask;
    unsigned long blue_mask = screen_image->image->blue_mask;

    unsigned char * image_buffer = (char *) malloc((width/scale)*(height/scale)*3*sizeof(unsigned char));

    long position = 0;

    for(int y=0; y < height; y=y+scale) {
        for(int x = 0; x < width; x=x+scale) {
            unsigned long red_sum, green_sum, blue_sum;
            red_sum = 0;
            green_sum = 0;
            blue_sum = 0;

            for(int s=0; s<scale; s++) {
                for(int s2=0; s2<scale; s2++) {
                    unsigned long pixel = XGetPixel(screen_image->image, x+s2, y+s);

                    uint8_t blue = pixel & blue_mask;
                    uint8_t green = (pixel & green_mask) >> 8;
                    uint8_t red = (pixel & red_mask) >> 16;

                    red_sum += red;
                    green_sum += green;
                    blue_sum += blue;
                }
            }

            image_buffer[position] = red_sum/(scale*scale);
            image_buffer[position+1] = green_sum/(scale*scale);
            image_buffer[position+2] = blue_sum/(scale*scale);

            position += 3;
        }
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    screen_frame->image = NULL;
    screen_frame->image_size = 0;

    jpeg_mem_dest(&cinfo, &screen_frame->image, &screen_frame->image_size);

    cinfo.image_width = (width/scale);
    cinfo.image_height = (height/scale);
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = (width/scale) * 3;	/* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    free(image_buffer);
}

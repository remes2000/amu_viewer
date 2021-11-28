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
#include <jpeglib.h>
#include <sys/types.h>
#include <unistd.h>


#define DEFAULT_TCP_PORT 1337
#define DEFAULT_UDP_PORT DEFAULT_TCP_PORT + 1
#define MAX_UDP_PACKET_SIZE 65535
#define DEAFULT_INTERFACE "enp0s31f6"
#define LOG(format, ...) printf("[%s] " format "\n", getFormattedTime(), ## __VA_ARGS__)
#define JPEG_QUALITY 20
#define JPEG_QUALITY_DOWNGRADE_STEP 3

char* getFormattedTime(void) {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    static char _retval[20];
    strftime(_retval, sizeof(_retval), "%Y-%m-%d %H:%M:%S", timeinfo);
    return _retval;
}

struct screen {
    Display * disp;
    Window root;
    int scr;
};

struct amu_viewer_setup {
    unsigned int access_code;
    int host_socket_tcp;
    int host_socket_udp;
    int client_socket_tcp;
    struct sockaddr_in tcp_host_address;
    struct sockaddr_in udp_host_address;
    struct sockaddr_in tcp_client_address;
    struct sockaddr_in udp_client_address;
    struct screen screen_details;
};

struct frame {
  unsigned char * image;
  unsigned long image_size;
};

socklen_t socklen = sizeof(struct sockaddr_in);
int setup(int argc, char **argv, struct amu_viewer_setup * viewer_setup);
int setup_host_address(int argc, char **argv, struct amu_viewer_setup * viewer_setup);
void setup_screen_details(struct amu_viewer_setup * viewer_setup);
void setup_access_code(struct amu_viewer_setup * viewer_setup);
int setup_host_sockets(struct amu_viewer_setup * viewer_setup);
int find_interface(char * interface, struct in_addr ** address);
void say_hello(struct amu_viewer_setup * viewer_setup);
void handle_connection(struct amu_viewer_setup * viewer_setup);
int authorize(struct amu_viewer_setup * viewer_setup);
int set_udp_client_address(struct amu_viewer_setup * viewer_setup);
int read_unsigned_int(int tcp_socket, unsigned int * dest);
int read_unsigned_short(int tcp_socket, unsigned short * dest);
int read_n_bytes(int tcp_socket, unsigned char * dest, unsigned int n);
int begin_screen_broadcast(struct amu_viewer_setup * viewer_setup, pid_t * broadcast_pid);
int send_frame(struct amu_viewer_setup * viewer_setup, int * jpeg_quality);
void send_transmission_over_udp_not_possible_message(struct amu_viewer_setup * viewer_setup);
void get_frame(struct frame * screen_frame, int quality, struct screen * screen_details);

int main(int argc, char **argv) {
    srand(time(NULL));
    struct amu_viewer_setup viewer_setup;

    if(setup(argc, argv, &viewer_setup) == -1) {
        printf("setup() failed \n");
        return -1;
    }
    say_hello(&viewer_setup);
    LOG("Waiting for connection...");
    while((viewer_setup.client_socket_tcp = accept(viewer_setup.host_socket_tcp, (struct sockaddr*) &viewer_setup.tcp_client_address, &socklen)) != -1) {
        handle_connection(&viewer_setup);
        close(viewer_setup.client_socket_tcp);
        LOG("Waiting for connection...");
    }
    printf("accept() failed");
    close(viewer_setup.client_socket_tcp);
    close(viewer_setup.host_socket_tcp);
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
    setup_screen_details(viewer_setup);
    setup_access_code(viewer_setup);
    return 0;
}

int setup_host_address(int argc, char **argv, struct amu_viewer_setup * viewer_setup) {
    unsigned short tcp_port = DEFAULT_TCP_PORT;
    unsigned short udp_port = DEFAULT_UDP_PORT;
    char * interface_name = DEAFULT_INTERFACE;
    struct in_addr * addr;
    if(argc > 1) {
        tcp_port = atoi(argv[1]);
    }
    if(argc > 2) {
        udp_port = atoi(argv[2]);
    }
    if(argc > 3) {
        interface_name = argv[3];
    }
    if(find_interface(interface_name, &addr) == -1) {
        printf("Cannot find interface with name %s \n", interface_name);
        return -1;
    }
    viewer_setup->tcp_host_address.sin_family = AF_INET;
    viewer_setup->tcp_host_address.sin_port = htons(tcp_port);
    // should i convert this?
    viewer_setup->tcp_host_address.sin_addr = *addr;
    viewer_setup->udp_host_address.sin_family = AF_INET;
    viewer_setup->udp_host_address.sin_port  = htons(udp_port);
    // should i convert this?
    viewer_setup->udp_host_address.sin_addr  = *addr;
    return 0;
}

int setup_host_sockets(struct amu_viewer_setup * viewer_setup) {
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

void setup_screen_details(struct amu_viewer_setup * viewer_setup) {
    viewer_setup->screen_details.disp = XOpenDisplay(0);
    viewer_setup->screen_details.scr = DefaultScreen(viewer_setup->screen_details.disp);
    viewer_setup->screen_details.root = DefaultRootWindow(viewer_setup->screen_details.disp);
}

void setup_access_code(struct amu_viewer_setup * viewer_setup) {
    viewer_setup->access_code = (rand() % 901) + 100;
    viewer_setup->access_code = 128;
}

int find_interface(char * interface_name, struct in_addr ** address) {
    struct ifaddrs *ifaddr, *ifa;
    struct sockaddr_in *sa;
    if(getifaddrs(&ifaddr) == -1) {
        return -1;
    }
    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
            if(strcmp(ifa->ifa_name, interface_name) == 0) {
                sa = (struct sockaddr_in *) ifa->ifa_addr;
                *address = &sa->sin_addr;
                return 0;
            }
        }
    }
    return -1;
}

void say_hello(struct amu_viewer_setup * viewer_setup) {
    printf("---=== AMU_VIEWER ===---- \n");
    printf("Listening on %s:%d \n", inet_ntoa(viewer_setup->tcp_host_address.sin_addr), ntohs(viewer_setup->tcp_host_address.sin_port));
    printf("UDP port: %d \n", ntohs(viewer_setup->udp_host_address.sin_port));
    printf("Access code = %d\n", viewer_setup->access_code);
}

void handle_connection(struct amu_viewer_setup * viewer_setup) {
    pid_t screen_broadcast_process;
    int authorization_result;

    LOG("Client %s connected", inet_ntoa(viewer_setup->tcp_client_address.sin_addr));
    authorization_result = authorize(viewer_setup);
    if(authorization_result == -1) {
        LOG("Authorization error");
        return;
    }
    if(authorization_result == 1) {
        if(send(viewer_setup->client_socket_tcp, &viewer_setup->udp_host_address.sin_port, sizeof(viewer_setup->udp_host_address.sin_port), 0) == -1) {
            LOG("An error occured while sending udp port to client");
            return;
        }
        if(set_udp_client_address(viewer_setup) == -1) {
            LOG("An error occured while reading client udp port");
            return;
        }
        if(begin_screen_broadcast(viewer_setup, &screen_broadcast_process) == -1) {
            LOG("An error occured while starting screen broadcast");
            return;
        }
        unsigned char * buf;
        while(1) {
            if(recv(viewer_setup->client_socket_tcp, buf, 1, 0) == 1) {
                LOG("Message from client: %x", buf);
            }
        }
    }
}

int authorize(struct amu_viewer_setup * viewer_setup) {
    uint8_t authorization_result;
    unsigned int provided_access_code;

    LOG("Waiting for access code...");
    if(read_unsigned_int(viewer_setup->client_socket_tcp, &provided_access_code) == -1) {
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
    if(send(viewer_setup->client_socket_tcp, &authorization_result, sizeof(authorization_result), 0) != sizeof(authorization_result)) {
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

int begin_screen_broadcast(struct amu_viewer_setup * viewer_setup, pid_t * broadcast_pid) {
    LOG("Begin screen broadcast, creating child process...");
    pid_t pid;
    if((pid = fork()) == -1) {
        LOG("fork() failed");
        return -1;
    }
    if(pid == 0) {
        int jpeg_quality = JPEG_QUALITY;
        while(send_frame(viewer_setup, &jpeg_quality) != -1) {
            usleep(300 * 1000);
        }
        exit(0);
    }
    *broadcast_pid = pid;
    LOG("Screen broadcast process pid = %d", pid);
    return 0;
}

int send_frame(struct amu_viewer_setup * viewer_setup, int * jpeg_quality) {
    struct frame screen_frame;
    get_frame(&screen_frame, *jpeg_quality, &viewer_setup->screen_details);
    if(screen_frame.image_size <= MAX_UDP_PACKET_SIZE) {
        sendto(viewer_setup->host_socket_udp,
               screen_frame.image,
               screen_frame.image_size,
               0,
               (struct sockaddr*) &viewer_setup->udp_client_address,
               sizeof(viewer_setup->udp_client_address));
    } else {
        LOG("Frame could not be sent because of too big frame size (%d). Downgrading jpeg quality from %d to %d", screen_frame.image_size, *jpeg_quality, *jpeg_quality-JPEG_QUALITY_DOWNGRADE_STEP);
        *jpeg_quality = *jpeg_quality - JPEG_QUALITY_DOWNGRADE_STEP;
    }
    free(screen_frame.image);
    if(*jpeg_quality < 0) {
        send_transmission_over_udp_not_possible_message(viewer_setup);
        LOG("Screen sharing impossible");
        return -1;
    }
    return 0;
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

void get_frame(struct frame * screen_frame, int quality, struct screen * screen_details) {
    /* x11 code */
    XWindowAttributes gwa;
    XGetWindowAttributes(screen_details->disp, screen_details->root, &gwa);
    int width = gwa.width;
    int height = gwa.height;
    XImage *image = XGetImage(screen_details->disp, screen_details->root, 0, 0, width, height, AllPlanes, ZPixmap);

    unsigned long red_mask = image->red_mask;
    unsigned long green_mask = image->green_mask;
    unsigned long blue_mask = image->blue_mask;

    unsigned char * image_buffer = (char *) malloc(width*height*3*sizeof(unsigned char));

    long position = 0;

    for(int y=0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);
            unsigned char blue = pixel & blue_mask;
            unsigned char green = (pixel & green_mask) >> 8;
            unsigned char red = (pixel & red_mask) >> 16;

            image_buffer[position] = red;
            image_buffer[position+1] = green;
            image_buffer[position+2] = blue;

            position += 3;
        }
    }

    /* libjpg code */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    screen_frame->image = NULL;
    screen_frame->image_size = 0;
    jpeg_mem_dest(&cinfo, &screen_frame->image, &screen_frame->image_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = width * 3;	/* JSAMPLEs per row in image_buffer */

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    free(image_buffer);
}

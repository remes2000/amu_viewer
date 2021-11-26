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

struct frame {
  unsigned char * image;
  unsigned long image_size;
};

socklen_t socklen = sizeof(struct sockaddr_in);
int setup(int argc, char **argv, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address, unsigned int * access_code, int * host_socket_tcp, int * host_socket_udp);
int setup_host_address(int argc, char **argv, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address);
int setup_access_code(unsigned int * access_code);
int setup_host_sockets(int * host_socket_tcp, int * host_socket_udp, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address);
int find_interface(char * interface, struct in_addr ** address);
void say_hello(struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address, unsigned int access_code);
void handle_connection(int host_socket_tcp, int host_socket_udp, int client_socket_tcp, struct sockaddr_in * tcp_client_address, unsigned int access_code);
int authorize(int client_socket_tcp, unsigned int access_code);
void set_udp_client_address(int client_socket_tcp, struct in_addr client_addr, struct sockaddr_in * udp_client_address);
unsigned int read_integer(int client_socket_tcp);
int begin_screen_broadcast(int host_socket_udp, struct sockaddr_in * udp_client_address);
int send_frame(int host_socket_udp, struct sockaddr_in * udp_client_address, int * jpeg_quality);
void get_frame(struct frame * screen_frame, int quality);

int main(int argc, char **argv) {
    struct sockaddr_in tcp_host_address, udp_host_address, tcp_client_address;
    unsigned int access_code;
    int host_socket_tcp, host_socket_udp, client_socket_tcp;
    srand(time(NULL));
    if(setup(argc, argv, &tcp_host_address, &udp_host_address, &access_code, &host_socket_tcp, &host_socket_udp) == -1) {
        printf("setup() failed \n");
        return -1;
    }
    say_hello(&tcp_host_address, &udp_host_address, access_code);
    LOG("Waiting for connection...");
    while((client_socket_tcp = accept(host_socket_tcp, (struct sockaddr*) &tcp_client_address, &socklen)) != -1) {
        handle_connection(host_socket_tcp, host_socket_udp, client_socket_tcp, &tcp_client_address, access_code);
        LOG("Waiting for connection...");
    }
    printf("accept() failed");
    close(client_socket_tcp);
    close(host_socket_tcp);
    close(host_socket_udp);
    return 0;
}

int setup(int argc, char **argv, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address, unsigned int * access_code, int * host_socket_tcp, int * host_socket_udp) {
    if(setup_host_address(argc, argv, tcp_host_address, udp_host_address) == -1) {
        return -1;
    }
    if(setup_host_sockets(host_socket_tcp, host_socket_udp, tcp_host_address, udp_host_address) == -1) {
        return -1;
    }
    setup_access_code(access_code);
    return 0;
}

int setup_host_address(int argc, char **argv, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address) {
    unsigned int tcp_port = DEFAULT_TCP_PORT;
    unsigned int udp_port = DEFAULT_UDP_PORT;
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
    tcp_host_address->sin_family = AF_INET;
    tcp_host_address->sin_port = htons(tcp_port);
    // should i convert this?
    tcp_host_address->sin_addr = *addr;
    udp_host_address->sin_family = AF_INET;
    udp_host_address->sin_port = htons(udp_port);
    // should i convert this?
    udp_host_address->sin_addr = *addr;
    return 0;
}

int setup_host_sockets(int * host_socket_tcp, int * host_socket_udp, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address) {
    *host_socket_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if(*host_socket_tcp == -1) {
        printf("setup_host_sockets() failed, on tcp socket() \n");
        return -1;
    }
    if(bind(*host_socket_tcp, (struct sockaddr*) tcp_host_address, sizeof(*tcp_host_address)) == -1) {
        printf("setup_host_sockets() failed, on tcp bind() \n");
        return -1;
    }
    if (listen(*host_socket_tcp, 1) == -1) {
        printf("setup_host_sockets() failed, on listen() \n");
        return -1; 
    }
    *host_socket_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if(*host_socket_udp == -1) {
        printf("setup_host_sockets() failed, on udp socket() \n");
        return -1;
    }
    if(bind(*host_socket_udp, (struct sockaddr*) udp_host_address, sizeof(*udp_host_address)) == -1) {
        printf("setup_host_sockets() failed, on udp bind() \n");
        return -1;
    }
    return 0;
}

int setup_access_code(unsigned int * access_code) {
    *access_code = (rand() % 901) + 100;
    *access_code = 128;
    return 0;
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

void say_hello(struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address, unsigned int access_code) {
    printf("---=== AMU_VIEWER ===---- \n");
    printf("Listening on %s:%d \n", inet_ntoa(tcp_host_address->sin_addr), ntohs(tcp_host_address->sin_port));
    printf("UDP port: %d \n", ntohs(udp_host_address->sin_port));
    printf("Access code = %d\n", access_code);
}

void handle_connection(int host_socket_tcp, int host_socket_udp, int client_socket_tcp, struct sockaddr_in * tcp_client_address, unsigned int access_code) {
    struct sockaddr_in udp_client_address;
    pid_t screen_broadcast_process;

    LOG("Client %s connected", inet_ntoa(tcp_client_address->sin_addr));
    if(authorize(client_socket_tcp, access_code) == 1) {
        set_udp_client_address(client_socket_tcp, tcp_client_address->sin_addr, &udp_client_address);
        screen_broadcast_process = begin_screen_broadcast(host_socket_udp, &udp_client_address);
        if(screen_broadcast_process == -1) {
            LOG("An error occured while starting screen broadcast");
            return;
        }
    }
}

int authorize(int client_socket_tcp, unsigned int access_code) {
    LOG("Waiting for access code...");
    unsigned int provided_access_code = read_integer(client_socket_tcp);
    LOG("Provided access code: %u", provided_access_code);
    if(provided_access_code == access_code) {
        LOG("Access granted");
        return 1;
    }
    LOG("Permission denied");
    return 0;
}

void set_udp_client_address(int client_socket_tcp, struct in_addr client_addr, struct sockaddr_in * udp_client_address) {
    LOG("Waiting for client udp port...");
    unsigned int client_udp_port = read_integer(client_socket_tcp);
    udp_client_address->sin_family = AF_INET;
    udp_client_address->sin_addr = client_addr;
    udp_client_address->sin_port = htons(client_udp_port);
    LOG("Client udp address: %s:%d", inet_ntoa(udp_client_address->sin_addr), client_udp_port);
}

unsigned int read_integer(int client_socket_tcp) {
    char buffer[sizeof(unsigned int)];
    int total_readed_bytes = 0;
    int readed_bytes = 0;
    while(total_readed_bytes < sizeof(unsigned int)) {
        readed_bytes = recv(client_socket_tcp, buffer + total_readed_bytes, sizeof(unsigned int) - total_readed_bytes, 0);
        if(readed_bytes == -1) {
            printf("Failure reading integer \n");
            exit(-1);
        }
        total_readed_bytes += readed_bytes;
    }
    return ntohl(*((unsigned int *) buffer));
}

int begin_screen_broadcast(int host_socket_udp, struct sockaddr_in * udp_client_address) {
    LOG("Begin screen broadcast, creating child process...");
    pid_t pid;
    if((pid = fork()) == -1) {
        LOG("fork() failed");
        return -1;
    }
    if(pid == 0) {
        int jpeg_quality = JPEG_QUALITY;
        while(send_frame(host_socket_udp, udp_client_address, &jpeg_quality) != -1) {
            usleep(300 * 1000);
        }
        exit(0);
    }
    printf("Child process %d \n", pid);
    return pid;
}

int send_frame(int host_socket_udp, struct sockaddr_in * udp_client_address, int * jpeg_quality) {
    struct frame screen_frame;
    get_frame(&screen_frame, *jpeg_quality);
    if(screen_frame.image_size <= MAX_UDP_PACKET_SIZE) {
        sendto(host_socket_udp, screen_frame.image, screen_frame.image_size, 0, (struct sockaddr*) udp_client_address, sizeof(*udp_client_address));
    } else {
        LOG("Frame could not be sent because of too big frame size (%d). Downgrading jpeg quality from %d to %d", screen_frame.image_size, *jpeg_quality, *jpeg_quality-JPEG_QUALITY_DOWNGRADE_STEP);
        *jpeg_quality = *jpeg_quality - JPEG_QUALITY_DOWNGRADE_STEP;
    }
    free(screen_frame.image);
    if(*jpeg_quality < 0) {
        LOG("Screen sharing impossible");
        return -1;
    }
    return 0;
}

void get_frame(struct frame * screen_frame, int quality) {
    /* x11 code */
    Display *disp;
    Window root;
    XWindowAttributes gwa;
    int scr;

    // the display points to X server
    disp = XOpenDisplay(0);
    // the screen refers to which screen of display to use
    scr = DefaultScreen(disp);
    // the window controls the actual window itself
    root = DefaultRootWindow(disp);

    XGetWindowAttributes(disp, root, &gwa);
    int width = gwa.width;
    int height = gwa.height;
    XImage *image = XGetImage(disp, root, 0, 0, width, height, AllPlanes, ZPixmap);

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

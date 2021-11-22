#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_TCP_PORT 1337
#define DEFAULT_UDP_PORT DEFAULT_TCP_PORT + 1
#define DEAFULT_INTERFACE "enp0s31f6"
#define LOG(format, ...) printf("[%s] " format "\n", getFormattedTime(), ## __VA_ARGS__)

char* getFormattedTime(void) {
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    static char _retval[20];
    strftime(_retval, sizeof(_retval), "%Y-%m-%d %H:%M:%S", timeinfo);
    return _retval;
}

socklen_t socklen = sizeof(struct sockaddr_in);
int setup(int argc, char **argv, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address, unsigned int * access_code, int * host_socket_tcp, int * host_socket_udp);
int setup_host_address(int argc, char **argv, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address);
int setup_access_code(unsigned int * access_code);
int setup_host_sockets(int * host_socket_tcp, int * host_socket_udp, struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address);
int find_interface(char * interface, struct in_addr ** address);
void say_hello(struct sockaddr_in * tcp_host_address, struct sockaddr_in * udp_host_address, unsigned int access_code);
void handle_connection(int host_socket_tcp, int host_socket_udp, int client_socket_tcp, struct sockaddr_in * tcp_client_address, unsigned int access_code);
int authorize(int client_socket_tcp, unsigned int access_code);
unsigned int read_access_code(int client_socket_tcp);

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
    LOG("Client %s connected", inet_ntoa(tcp_client_address->sin_addr));
    if(authorize(client_socket_tcp, access_code) == 1) {
        
    }
}

int authorize(int client_socket_tcp, unsigned int access_code) {
    LOG("Waiting for access code...");
    unsigned int provided_access_code = read_access_code(client_socket_tcp);
    LOG("Provided access code: %u", provided_access_code);
    if(provided_access_code == access_code) {
        LOG("Access granted");
        return 1;
    }
    LOG("Permission denied");
    return 0;
}

unsigned int read_access_code(int client_socket_tcp) {
    char buffer[sizeof(unsigned int)];
    int total_readed_bytes = 0;
    int readed_bytes = 0;
    while(total_readed_bytes < sizeof(unsigned int)) {
        readed_bytes = recv(client_socket_tcp, buffer + total_readed_bytes, sizeof(unsigned int) - total_readed_bytes, 0);
        if(readed_bytes == -1) {
            printf("Failure on read access code \n");
            exit(-1);
        }
        total_readed_bytes += readed_bytes;
    }
    return ntohl(*((unsigned int *) buffer));
}
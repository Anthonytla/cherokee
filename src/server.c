#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include "../include/server.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/request_handler.h"
#include "../include/cache.h"




int main(int argc, char **argv) {
    if (argc < 2) {return 2;}
    spawn_process(argv[1]);  
    waitForAll();
    puts("end");
}

int init_server(char *port) {
    int status;
    struct addrinfo hints;
    struct addrinfo* servinfo;  // will point to the results
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me



    if ((status = getaddrinfo("0.0.0.0", port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    int sockfd;
    // make a socket:
    check(sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol), "Failed to create socket");

    /*int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }

    flags |= O_NONBLOCK;
    int result = fcntl(sockfd, F_SETFL, flags);
    if (result == -1) {
        return -1;
    }*/
    int yes=1;
    //char yes='1'; // Solaris people use this

    // lose the pesky "Address already in use" error message
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
        perror("setsockopt");
        exit(1);
    }
    check(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen), "Failed to bind socket");
    // listen to a port
    check(listen(sockfd, 1000), "Failed to listen");
    freeaddrinfo(servinfo);
    return sockfd;
}

int check(int status, char * message) {
    if (status < 0) {
        perror(message);
        return 0;
    }
    return 1;
}

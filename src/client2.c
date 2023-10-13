#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <readline/readline.h>
#include "../include/client2.h"

int main(int argc, char **argv) {
    if (argc < 2) {return 2;}
    int status;
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if ((status = getaddrinfo("0.0.0.0", argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // make a socket:
    check(sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol), "Failed to create socket");

    check(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen), "Failed to connect");

 
    while(1) {
        handleData(sockfd);
    }
    

    
    close(sockfd);

    //close(sockfd);
    freeaddrinfo(servinfo);
}

int check(int status, char * message) {
    if (status < 0) {
        perror(message);
        return 0;
    }
    return 1;
}

void handleData(int sockfd) {
    char *line = malloc(1024);
    char request[8097];
    // size_t len = 0;

    memset(request, '\0', 2048);
    memset(line, '\0', 1024);
    // char lineSize = 0;

    // while((line = fgets(line, 1024, stdin)) != NULL) {
    //     if (strstr(request, "\n\n")) {break;}
    //     memcpy(request, line, strlen(line));
    //     puts(request);
    // }

    fgets(request, 1024, stdin);

    

    //printf("%s\n", request);
    check(send(sockfd, request, strlen(request)+1, 0), "Send failed");
    char buffer[8097];
    memset(buffer, '\0', 8097);
    while (recv(sockfd, buffer, 8097, 0) > 0) {
        puts(buffer); 
        memset(buffer, '\0', 8097);
    }
    
    //free(line);
}


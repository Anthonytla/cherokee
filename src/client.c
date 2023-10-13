#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <readline/readline.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include "../include/client.h"

#define CLIENT_MODE 0
int main(int argc, char **argv) {
    if (argc < 3) {return 2;}
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

 
    char buff[8097];
    memset(buff, '\0', 8097);
    handleData(sockfd, argv[2]);
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    select(sockfd+1, &readfds, NULL, NULL, NULL);
    check(recv(sockfd, buff,8097, 0), "Recv failed");
    int fd = open("recv.jpg", O_WRONLY | O_CREAT | O_TRUNC, 0700);
    check(write(fd, buff, 8097), "write failed");
    printf("%s\n", buff);
        


    //close(sockfd);
    freeaddrinfo(servinfo);
}

int check(int status, char *message)
{
  if (status < 0)
  {
    perror(message);
    return 0;
  }
  return 1;
}

void handleData(int sockfd, char *filename) {
    char request[8097];
    // size_t len = 0;

    int fd = open(filename, O_RDONLY);

    memset(request, '\0', 8097);
    // char lineSize = 0;

    int r = read(fd, request, 8097);
    if (r < 0) {perror("read failed");}
    check(send(sockfd, request, strlen(request), 0), "Send failed");

    
    //free(line);
}

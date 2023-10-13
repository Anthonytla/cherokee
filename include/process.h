#ifndef __PROCESS_H__
#define __PROCESS_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "../include/cache.h"
#include "../include/queue.h"
#include "../include/request.h"
#include "../include/response.h"
#include <errno.h>

#define MAX_CONNECTION 50

struct Sharedfd {
    fd_set *readfds;
    fd_set *writefds;
};

struct Task {
    int client_socket;
    int fileFd;
    char *request;
    struct Request request_struct;
    struct Response response;
    struct Sharedfd sharedfd;
};

void spawn_process(char *port);
void waitForAll();
void watch(int server_socket);
void* responseBuildThread();
void* requestParsingThread() ;
void* responseBuildThreadPut(void* arg);
int countFileDescriptors(fd_set fds);

#endif

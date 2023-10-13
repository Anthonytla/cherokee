#ifndef __SERVER_H__
#define __SERVER_H__
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "../include/process.h"
#include "../include/queue.h"

int init_server(char *port);
int check(int status, char * message);
#endif

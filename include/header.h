#ifndef __HEADER_H__
#define __HEADER_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
// #include "../include/request.h"
#include "../include/response.h"

struct ContentTypeHeader
{
  char *media_type;
};



struct Header {
    struct ContentTypeHeader *content_type_header;
    struct media *accept_header;
};

// int checkFileExt(struct Request request, char **filename);
int checkFolderExist(char *foldername);
// int checkAccept(struct Request request, char *filename);
char* generateResponseHeader(const char* request_uri, int statusCode, char* statusMessage,  struct Response *response);
void parse_content_type(char *h, struct Header *header) ;
char* getContentType(const char* request_uri);

// void send_error(int client_socket, int err);
#endif

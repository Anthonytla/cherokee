#include "../include/header.h"
#include "../include/request.h"
#include "../include/media_struct.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int checkAccept(struct Request request, char *filename) {
  char *ext = strrchr(filename, '.');
  if (!ext) {return 1;}
  if (!find(request.header.accept_header, "*/*") && !find(request.header.accept_header, ext + 1)) {return 0;}
  return 1;
}

int checkFileExt(struct Request request, char **filename) {
  if (opendir(*filename)) {return 0;}
  char *ext = strrchr(*filename, '.');
  FILE *file;
  if (!ext) {
    struct media *tmp = request.header.accept_header;
    while(tmp){
      char *file_tmp = malloc(strlen(*filename+1) + 7);
      strcpy(file_tmp, *filename);
      strcat(file_tmp, ".");
      strcat(file_tmp, strrchr(tmp->media, '/') + 1);

      *filename = file_tmp;
      if ((file = fopen(file_tmp, "rb"))) {return fileno(file);}
      tmp = tmp->next;
      
    }
    return 0;
  }
  else {
    return fileno(fopen(*filename+1,"rb"));
  }
}

char* generateResponseHeader(const char* request_uri, int statusCode, char* statusMessage, struct Response *response) {
    char* contentType = getContentType(request_uri);
    set_headers(contentType, response);
    set_response_status_line(statusCode, statusMessage, response);
    return NULL;
} 


char* getStatusMessage(int statusCode) {
    switch (statusCode) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 409:
            return "Conflict";
        case 500:
            return "Internal Server Error";
        default:
            return "Unknown";
    }
}

char* getContentType(const char* request_uri) {
    if (!request_uri) {return NULL;}
    char *ext = strrchr(request_uri, '.');
    if (!ext) {return "text/plain";}
    if (!strcmp(ext, ".txt")) {return "text/plain";}
    if (!strcmp(ext, ".html")) {return "text/html";}
    if (!strcmp(ext, ".css")) {return "text/css";}
    if (!strcmp(ext, ".js")) {return "text/javascript";}
    if (!strcmp(ext, ".jpg")) {return "image/jpeg";}
    if (!strcmp(ext, ".png")) {return "image/png";}
    if (!strcmp(ext, ".json")) {return "application/json";}
    if (!strcmp(ext, ".pdf")) {return "application/pdf";}
    if (!strcmp(ext, ".xml")) {return "application/xml";}
    return "unknown format";
}

int errno_to_status(int errnoValue) {
    switch (errnoValue) {
        case ENOENT:
            return 404; // Not Found
        case EACCES:
            return 403; // Forbidden
        case EEXIST:
            return 409; // Conflict
        case EINVAL:
            return 400; // Bad Request
        case ENOSPC:
            return 507; // Insufficient Storage
        case ECONNREFUSED:
            return 503; // Service Unavailable
        // Add more mappings as needed
        default:
            return 500; // Internal Server Error (if no specific mapping found)
    }
}

void parse_content_type(char *h, struct Header *header) {
  if (!strstr(h, "Content-Type")) {return;}
  strtok_r(h, ":", &h);
  char *media_type = strtok_r(h, ":", &h);
  struct ContentTypeHeader *content_type_header = malloc(sizeof(struct ContentTypeHeader));
  content_type_header->media_type = media_type;
  header->content_type_header = content_type_header;
}
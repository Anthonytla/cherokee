#include "../include/request.h"
#include "../include/header.h"
#include "../include/media_struct.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

size_t get_message_body_length(char *request_headers) {
  // char *tmp = strdup(request_headers);
  // size_t size = strlen(request_headers);
  // char tmp[size];
  // bzero(tmp, size);
  // memcpy(tmp, request_headers, size);
  //   // Iterate through the request headers to find the Content-Length header
    char *h = strtok_r(request_headers, "\n", &request_headers);
    while (h != NULL) {
        if (strstr(h, "Content-Length")) {
            strtok_r(h, ":", &h);
            return atoi(h);
        }
        h = strtok_r(NULL, "\n", &request_headers);
    }

    return 0;  // Default value if Content-Length header is not found or has an invalid value
}

void parse_request(char *request, struct Request *request_struct)
{
  if (!request) {return;}
  // struct Request *request_struct = malloc(sizeof(struct Request));
  //char *request_line_str = strtok(request, "\n");
  //char *headers_body = strtok(NULL, "\n");
  char *request_headers = get_request_headers(request);
  
  char *message_body = get_message_body(request);
  size_t bodyLength  = get_message_body_length(request);

  // char *request_line_str = strtok_r(request_headers, "\r\n", &request_headers);
  // if (!request_line_str) {return;}
  // parse_request_line(request_line_str, request_struct);
  if (!request_headers) {return;}
  char *headers_copy = strdup(request_headers);
  char *request_line_str = strtok(headers_copy, "\r\n");
  
  // struct Header *header = malloc(sizeof(struct Header));
  struct Header header;
  char *h = NULL;
  while ((h = strtok(NULL, "\r\n")))
  {
    parse_content_type(h, &header);
    // parse_accept(h, header);
  }
  
  parse_request_line(request_line_str, request_struct);
  request_struct->message_body = message_body;
  request_struct->header = header;
  request_struct->body_length = bodyLength;
  free(request);
  free(request_headers);
  free(headers_copy);
}

void parse_request_line(char *request_line_str, struct Request *request)
{
  struct RequestLine request_line;
  if (!request_line_str) {return ;}
  char *saveptr = NULL;
  strcpy(request_line.method, strtok_r(request_line_str, " ", &saveptr));
  strcpy(request_line.request_uri, strtok_r(NULL, " ", &saveptr));
  strcpy(request_line.http_version, strtok_r(NULL, " ", &saveptr));
  request->request_line = request_line;
}

void parse_headers(char *headers_str, struct Header *header)
{
  // struct Headers *headers = malloc(sizeof(struct Headers));
  // struct Header *header = malloc(sizeof(struct Header));
  // char *saveptr1 = NULL;
  char *h = NULL;
  while ((h = strtok_r(headers_str, "\n", &headers_str)))
  {
    parse_content_type(h, header);
    parse_accept(h, header);
  }
}

  char *get_request_headers(char *request)
  {
    // Create a copy of the input string
    //char remaining[strlen(request)];
    
    char *token = strstr(request, "\r\n\r\n");
    if (token == NULL) {
        // The "\r\n\r\n" sequence was not found, so the headers are missing.
        return NULL;
    }
    char *remaining = strndup(request, strlen(request) - strlen(token));
    //*token = '\0'; // Replace the delimiter with null character
    return remaining;
    // char *header = strtok_r(request, "\n\n", &request);
    // return header;
  }

char *get_message_body(char *request)
{
  //char *remaining = strdup(request); // Create a copy of the input string
  char *token = strstr(request, "\r\n\r\n");
  if (!token)
  {
    return NULL;
  }
  return token + strlen("\r\n\r\n");
}

int is_get_method(struct Request request)
{
  return !strcmp(request.request_line.method, "GET");
}

int is_head_method(struct Request request)
{
  return !strcmp(request.request_line.method, "HEAD");
}

int is_put_method(struct Request request)
{
  return !strcmp(request.request_line.method, "PUT");
}

int is_post_method(struct Request request)
{
  return !strcmp(request.request_line.method, "POST");
}

int is_delete_method(struct Request request)
{
  return !strcmp(request.request_line.method, "DELETE");
}

int is_custom_method(struct Request request)
{
  return !strcmp(request.request_line.method, "CUSTOM");
}


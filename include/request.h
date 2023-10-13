#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <stdlib.h>
#include "../include/header.h"
struct RequestLine
{
  char method[10];
  char request_uri[255];
  char http_version[10];
};

struct Request
{
  struct RequestLine request_line;
  char *message_body;
  struct Header header;
  size_t body_length;  // New member for storing the length of the message body
};

void parse_request(char *request, struct Request *request_struct);
void parse_request_line(char *request_line_str, struct Request *request);
void parse_headers(char *headers_str, struct Header *header);
int is_get_method(struct Request request);
int is_head_method(struct Request request);
int is_put_method(struct Request request);
int is_post_method(struct Request request);
int is_delete_method(struct Request request);
int is_custom_method(struct Request request);
int check_uri_segment(char *uri_segment, int position);
char *get_request_headers(char *request);
char *get_message_body(char *request);
size_t get_message_body_length(char *request_headers);
char *get_request_line(char *request);
#endif

#ifndef __RESPONSE_H__
#define __RESPONSE_H__
#include <stddef.h>
#include <stdlib.h>
struct StatusLine
{
  char *http_version;
  int status_code;
  char *reason_phrase;
};

struct Response
{
  char *header;
  size_t content_length;
  struct StatusLine status_line;
  char *message_body;
};

char *build(struct Response response);
void set_response_status_line(int status_code, char *reason_phrase, struct Response *response);
void set_headers(char *header, struct Response *response);
int errno_to_status(int errnoValue);
#endif

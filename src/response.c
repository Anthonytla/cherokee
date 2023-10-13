#include "../include/response.h"
#include <string.h>
#include <stdio.h>

int check_uri_segment(char *uri_segment, int position)
{
  if (position == 0 && strcmp(uri_segment, "http:"))
  {
    return 0;
  }
  else if (position == 1 && strcmp(uri_segment, "cherokee"))
  {
    return 0;
  }
  return 1;
}

char *build(struct Response response)
{
  char *buffer = malloc(8097);
  memset(buffer, '\0', 8097);
  sprintf(buffer, "{\n  Status code: %d\n  Reason: %s\n  Message: %s\n}", response.status_line.status_code, response.status_line.reason_phrase, response.message_body);
  return buffer;
}

void set_response_status_line(int status_code, char *reason_phrase, struct Response *response)
{
  struct StatusLine status_line;
  status_line.http_version = "HTTP/1.1";
  status_line.status_code = status_code;
  status_line.reason_phrase = reason_phrase;
  response->status_line = status_line;
}

void set_headers(char *header, struct Response *response) {
  if (!header) {return;}
  response->header = strdup(header);
}

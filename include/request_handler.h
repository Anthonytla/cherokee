#ifndef __REQUEST_HANDLER_H__
#define __REQUEST_HANDLER_H__
#include "../include/request.h"
#include "../include/response.h"
int get(struct Request request, struct Response *response);
void head(struct Request request, struct Response *response);
int put(struct Request request, struct Response *response);
int post(struct Request request, struct Response *response);
int delete(struct Request request, struct Response *response);
int custom(struct Request request, struct Response *response);
int createFileWithBody(const char* filePath, const char* extension,int forPost, struct Response *response) ;
char *trim_whitespace(const char *str);
char *extract_extension(const char *media_type);
//struct Response *request_handling(char *request, char **cache);
//void* requestParsingThread(void* arg) ;

#endif

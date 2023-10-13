#ifndef __CLIENT_H__
#define __CLIENT_H__

int check(int status, char * message);
void handleData(int sockfd, char *filename);

#endif

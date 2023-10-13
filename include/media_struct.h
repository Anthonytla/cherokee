#ifndef __MEDIA_STRUCT_H__
#define __MEDIA_STRUCT_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/header.h"
struct media
{
    float q;
    char *media;
    struct media *next;
};

struct media * prepend(struct media *list, char *media, float q);
int find(struct media *list ,char *media);
struct media *append(struct media *list, char *media, float q);
struct media* add_sort(struct media *list, char *media, float q);
void medias_print(struct media *list);
void parse_accept(char * request, struct Header *header);
char *get_q_value(char *q);
struct media *parse_media(char *accept);
#endif

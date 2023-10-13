#include "../include/media_struct.h"
struct media * prepend(struct media *list, char *media, float q) 
{
    struct media *new = malloc(sizeof(struct media));
    if (!new) {return NULL;}
    new->next = list;
    new->media = malloc(strlen(media) + 1);
    strcpy(new->media, media);
    new->q = q;
    return new;
    
}

struct media *append(struct media *list, char *media, float q) {
    struct media *new = malloc(sizeof(struct media));
    if (!new) {return NULL;}
    struct media *tmp = list;
    while (tmp->next) {
        tmp = tmp->next;
    }
    new->media = malloc(sizeof(media));
    strcpy(new->media, media);
    new->q = q;
    tmp->next = new;
    return list;


}
struct media* add_sort(struct media *list, char *media, float q){
    if (!list) {
        return prepend(list, media, q);

    }
    struct media *tmp = list;
    if (tmp->q < q) {
        return prepend(list, media, q);
    }
    while (tmp->next && tmp->next->q > q) {
        tmp = tmp->next;
    }
    if (!tmp) {
        
        return append(list, media, q);
    }
    
    struct media *nn = malloc(sizeof(struct media));
    nn->media = malloc(sizeof(media));
    strcpy(nn->media, media);
    nn->q = q;
    nn->next = tmp->next;
    tmp->next = nn;
    return list;
}

int find(struct media *list ,char *media) {
    if (!list) {return 0;}
    while (list) {
        if (!strstr(list->media, media)){list=list->next ;continue;}
        return 1;
    }
    return 0;
}

void medias_print(struct media *list) {
    struct media *tmp = list;
    while (tmp) {
        printf("media: %s- quality:%f\n", tmp->media, tmp->q);
        tmp = tmp->next;
    }
}

void parse_accept(char * request, struct Header *header) {
    if (!strstr(request, "Accept")) {return;}
    strtok(request, ":");
    char *accept = strtok(NULL, ":");
    header->accept_header = parse_media(accept);
}
char *get_q_value(char *q)  {
    strtok(q,"=");
    return strtok(NULL, "=");
}
struct media *parse_media(char *accept) {
    char *end_ptr = NULL;
    struct media *medias = NULL;
    char *saveptr1 = NULL; 
    char *saveptr2 = NULL;
    for (char *str1 = accept ,j = 0 ; ;j++,str1=NULL) {
        char *token = strtok_r(str1, ",", &saveptr1);
        if (!token) {
            break;
        }
        char *media=strtok_r(token, ";", &saveptr2);
        char *q = strtok_r(NULL,";",&saveptr2);
        if (!q)
        {medias = add_sort(medias, media, strtof("1", &end_ptr));}
        else {
            medias = add_sort(medias, media, strtof(get_q_value(q), &end_ptr));
        }
       
    }
    return medias;
}

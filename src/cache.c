#include "../include/cache.h"
#include <stdlib.h>
#include <stddef.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "../include/response.h"
#include <math.h>
#include "../include/header.h"
#define LRU_SIZE 1000000
struct LRUCache *init_cache() {
    struct LRUCache *lru_cache =  mmap(NULL, sizeof(struct LRUCache), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (!lru_cache){perror("mmap failed");}
    lru_cache->cache = malloc(LRU_SIZE*sizeof(struct StackItem));
    // for (int i = 0; i < LRU_SIZE; i++) {
    //     lru_cache->cache[i] = malloc(sizeof(struct StackItem));
    // }
    lru_cache->stack = stack_init();
    return lru_cache;

}

struct Stack *stack_init(void)
{
    struct Stack *stack = malloc(sizeof(struct Stack));
    stack->size = 0;
    stack->head = NULL;
    stack->tail = NULL;
    return stack;
}
int stack_push_front(struct Stack *stack, struct StackItem *new_node)
{
    //struct StackItem *head = malloc(sizeof(struct StackItem));
    // if (!head)
    //     return 0;
    // head->value = strdup(value);
    new_node->prev = NULL;
    new_node->next = stack->head;
    if (stack->head)
        stack->head->prev = new_node;
    stack->head = new_node;
    if (!stack->tail)
        stack->tail = stack->head;
    stack->size++;
    return 1;
}

int stack_pop_back(struct Stack *stack, struct StackItem *to_remove) {
    if (to_remove->prev != NULL) {
        to_remove->prev->next = to_remove->next;
    } else {
        stack->head = to_remove->next;
    }

    if (to_remove->next != NULL) {
        to_remove->next->prev = to_remove->prev;
    } else {
        stack->tail = to_remove->prev;
    }
    return 1;
}

unsigned long hash(char *str)
{
    
    int length = strlen(str);
    int res = 0;
    for (int i = 0; i < length; i++) {
        res += pow(2, length-i)*str[i];
    }
    return res % LRU_SIZE;

    // unsigned long hash = 5381;
    // int c;

    // while ((c = *str++))
    //     hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    // return hash%LIMIT;
}
// void print_cache(char **cache) {
//     // for (int i = 0; i < 1000; i++) {
//     //     if (cache[i][0] != '\0'
//     // }
// }
void put_cache(char *uri, char *value, struct LRUCache *lru_cache){
    unsigned int key = hash(uri);
    if (lru_cache->cache[key]) // Key Already Exist, just update the value and move it to top
    {
        struct StackItem *item = lru_cache->cache[key];
        item->value = strdup(value);
        stack_pop_back(lru_cache->stack, item);
        stack_push_front(lru_cache->stack, item);
    } else {
        struct StackItem *newnode = malloc(sizeof(struct StackItem));
        newnode->prev = NULL;
        newnode->next = NULL;
        newnode->value = strdup(value);
        newnode->key = key;
        if (lru_cache->stack->size > LRU_SIZE) // We have reached maxium size so need to make room for new element.
        {
            lru_cache->cache[lru_cache->stack->tail->key] = NULL;
            stack_pop_back(lru_cache->stack, lru_cache->stack->tail);				
            stack_push_front(lru_cache->stack, newnode);

        } else {
            stack_push_front(lru_cache->stack, newnode);
        }

        lru_cache->cache[key] = malloc(sizeof(struct StackItem));
        lru_cache->cache[key] = newnode;
    }
}
char *look_up(char *uri, struct LRUCache *lru_cache) {
    
   // char *to_search = strstr(uri, "cherokee") + 9;
    unsigned int index = hash(uri);
    if (lru_cache->cache[index] && strlen(lru_cache->cache[index]->value) > 0) {
        // struct StackItem *new = malloc(sizeof(struct StackItem));
        // new->prev = NULL;
        // new->next = NULL;
        // if (lru_cache->stack->size > LRU_SIZE) {
        //     lru_cache->cache[hash()]
        //     stack_pop_back(lru_cache->stack, lru_cache->stack->tail);
        //     stack_push_front(lru_cache, new);
        // }
        stack_pop_back(lru_cache->stack, lru_cache->cache[index]);
        stack_push_front(lru_cache->stack, lru_cache->cache[index]);
        return lru_cache->cache[index]->value;
    }
    else {

        return NULL;
    }
}

char *load_from_cache(struct Request request, struct LRUCache *lru_cache) {
   
    if (!is_get_method(request)) {return 0;};
    if (strstr(getContentType(request.request_line.request_uri), "image") != NULL) {return 0;}
    char *message = look_up(request.request_line.request_uri, lru_cache);
    if (!message) {return NULL;}
    //set_response_status_line(200, "Fetched from cache", response);
    return message;
}

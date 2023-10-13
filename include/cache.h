#ifndef __CACHE_H__
#define __CACHE_H__

#define LIMIT 5

#include "../include/response.h"
#include "../include/request.h"

struct Stack {
    struct StackItem *head;
    struct StackItem *tail;
    size_t size;
};
struct StackItem {
    char *value;
    unsigned int key;
    struct StackItem *prev;
    struct StackItem *next;
};
struct LRUCache {
    struct StackItem* *cache;
    struct Stack *stack;
};
unsigned long hash(char *str);
char *look_up(char *uri, struct LRUCache *cache);
struct LRUCache *init_cache();
char *load_from_cache(struct Request request, struct LRUCache *lru_cache);
void put_cache(char *uri, char *value, struct LRUCache *lru_cache);
struct Stack *stack_init(void);
int stack_push_front(struct Stack *stack, struct StackItem* new_node);
int stack_push_back(struct Stack *stack, char* value);
#endif

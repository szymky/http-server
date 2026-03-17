
#ifndef HASH_MAP_H
#define HASH_MAP_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 16
#define LOAD_FACTOR_THRESHOLD 0.75


typedef void (*router_handler)(int);

typedef struct Node {
    char *key;
    router_handler value;
    struct Node *next;
} Node;

typedef struct {
    Node **buckets;
    int size;
    int count;
} HashMap;

unsigned int hashmap_hash(const char *key, int size);
HashMap *hashmap_create();
void hashmap_resize(HashMap *map);
void hashmap_put(HashMap *map, const char *key, router_handler value);
router_handler hashmap_get(HashMap *map, const char *key);
void hashmap_delete(HashMap *map, const char *key);
void hashmap_free(HashMap *map);


#endif // !HASH_MAP_H

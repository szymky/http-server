
#include "HashMap.h"
#include <stdlib.h>
#include <string.h>

unsigned int hashmap_hash(const char *key, int size) {
    unsigned int h = 0;
    while (*key) {
        h = (h * 31) + *key;
        key++;
    }
    return h % size;
}
HashMap *hashmap_create() {
    HashMap *map = malloc(sizeof(HashMap));
    map->size = INITIAL_SIZE;
    map->count = 0;
    map->buckets = calloc(map->size, sizeof(Node*));
    return map;
}
void hashmap_resize(HashMap *map) {
    int old_size = map->size;
    Node **old_buckets = map->buckets;

    map->size *= 2;
    map->buckets = calloc(map->size, sizeof(Node*));
    map->count = 0;

    for (int i = 0; i < old_size; i++) {
        Node *current = old_buckets[i];
        while (current) {
            Node *next = current->next;

            unsigned int index = hashmap_hash(current->key, map->size);
            current->next = map->buckets[index];
            map->buckets[index] = current;
            current = next;
            map->count++;
        }
    }

    free (old_buckets);
}
void hashmap_put(HashMap *map, const char *key, router_handler value) {
    if ((float)(map->count + 1) / map->size > LOAD_FACTOR_THRESHOLD) {
        hashmap_resize(map);
    }

    unsigned int index = hashmap_hash(key, map->size);
    Node *current = map->buckets[index];

    while(current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    Node *new_node = malloc(sizeof(Node));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    map->count++;
}
router_handler hashmap_get(HashMap *map, const char *key) {
    unsigned int index = hashmap_hash(key, map->size);
    Node *current = map->buckets[index];

    while(current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}
void hashmap_delete(HashMap *map, const char *key) {
    unsigned int index = hashmap_hash(key, map->size);
    Node *current = map->buckets[index];
    Node *prev = NULL;

    while(current) {
        if (strcmp(current->key, key) == 0) {
            if (prev){
                prev->next = current->next;
            } else {
                map->buckets[index] = current->next;
            }

            free(current->key);
            free(current);
            map->count--;
            return;
        }
        prev = current;
        current = current->next;
    }
}
void hashmap_free(HashMap *map) {
    for (int i = 0; i < map->size; i++) {
        Node *current = map->buckets[i];
        while(current) {
            Node *tmp = current;
            current = current->next;
            free(tmp->key);
            free(tmp);
        }
    }
    free(map->buckets);
    free(map);
}

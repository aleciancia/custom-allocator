#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H

#include <stddef.h> 

typedef struct block_header {
    size_t size;                
    int is_free;               
    struct block_header *next; 
} block_header_t;


#define HEADER_SIZE sizeof(block_header_t)

void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t size);

void print_memory_map();

#endif 
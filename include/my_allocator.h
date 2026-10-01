#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>


#define MAGIC 0xDEADBEEF

typedef struct block_header {
    uint32_t magic;             
    int is_free;               
    size_t size;                
    struct block_header *next;
} block_header_t;


#define HEADER_SIZE ((sizeof(block_header_t) + 15) & ~15)

void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t size);
void print_memory_map();

#ifdef BENCH
extern unsigned long coalescing_count;
extern size_t sbrk_bytes_total;
#endif

#endif 
#ifndef MY_ALLOCATOR_H
#define MY_ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>

// MAGIC NUMBER: Una firma per riconoscere i nostri blocchi e rilevare puntatori non validi
#define MAGIC 0xDEADBEEF

typedef struct block_header {
    uint32_t magic;             // Identificatore univoco del blocco
    int is_free;                // 1 se libero, 0 se occupato
    size_t size;                // Dimensione payload
    struct block_header *next;
} block_header_t;

// Calcoliamo la dimensione dell'header forzando un allineamento rigoroso a 16 byte
#define HEADER_SIZE ((sizeof(block_header_t) + 15) & ~15)

void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t size);
void print_memory_map();

#endif // MY_ALLOCATOR_H
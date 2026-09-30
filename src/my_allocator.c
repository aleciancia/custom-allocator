#include "my_allocator.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

// La variabile globale DEVE stare qui in alto
static block_header_t *head = NULL;

// Funzione di debug per stampare la mappa
void print_memory_map() {
    block_header_t *current = head;
    int count = 0;
    printf("--- MAPPA DELLA MEMORIA ---\n");
    if (!current) {
        printf("Heap vuoto.\n");
    }
    while (current) {
        printf("Blocco %d: Size = %zu bytes | Free = %d | Indirizzo = %p\n",
               count, current->size, current->is_free, (void*)current);
        current = current->next;
        count++;
    }
    printf("---------------------------\n\n");
}

static size_t align8(size_t size) {
    return (size + 7) & ~7;
}

static block_header_t *find_free_block(size_t size) {
    block_header_t *current = head;
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void split_block(block_header_t *block, size_t size) {
    if (block->size >= size + HEADER_SIZE + 8) {
        block_header_t *new_block = (block_header_t *)((uint8_t *)block + HEADER_SIZE + size);
        new_block->size = block->size - size - HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
    }
}

static block_header_t *request_space(block_header_t *last, size_t size) {
    block_header_t *block = sbrk(0);
    void *request = sbrk(size + HEADER_SIZE);
    
    if (request == (void *)-1) {
        return NULL;
    }
    
    if (last) {
        last->next = block;
    }
    
    block->size = size;
    block->is_free = 0;
    block->next = NULL;
    
    return block;
}

void *my_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    size = align8(size);
    block_header_t *block;
    
    if (!head) {
        block = request_space(NULL, size);
        if (!block) return NULL;
        head = block;
    } else {
        block = find_free_block(size);
        if (block) {
            block->is_free = 0;
            split_block(block, size);
        } else {
            block_header_t *last = head;
            while (last->next) {
                last = last->next;
            }
            block = request_space(last, size);
            if (!block) return NULL;
        }
    }
    
    return (block + 1);
}

static void coalesce() {
    block_header_t *current = head;
    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            current->size += HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void my_free(void *ptr) {
    if (!ptr) {
        return;
    }
    
    block_header_t *block = (block_header_t *)ptr - 1;
    block->is_free = 1;
    
    coalesce();
}

void *my_calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void *ptr = my_malloc(total_size);
    
    if (ptr) {
        uint8_t *p = (uint8_t *)ptr;
        for (size_t i = 0; i < total_size; i++) {
            p[i] = 0;
        }
    }
    
    return ptr;
}

void *my_realloc(void *ptr, size_t size) {
    if (!ptr) {
        return my_malloc(size);
    }
    
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)ptr - 1;
    
    if (block->size >= size) {
        return ptr;
    }
    
    void *new_ptr = my_malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    
    uint8_t *src = (uint8_t *)ptr;
    uint8_t *dest = (uint8_t *)new_ptr;
    for (size_t i = 0; i < block->size; i++) {
        dest[i] = src[i];
    }
    
    my_free(ptr);
    return new_ptr;
}
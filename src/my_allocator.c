#include "my_allocator.h"
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>

static block_header_t *head = NULL;

// FIX BUG 6: Allineamento a 16 byte (max_align_t)
static size_t align16(size_t size) {
    return (size + 15) & ~15;
}

void print_memory_map() {
    block_header_t *current = head;
    int count = 0;
    printf("--- MAPPA DELLA MEMORIA ---\n");
    if (!current) printf("Heap vuoto.\n");
    while (current) {
        printf("Blocco %d: Size = %zu bytes | Free = %d | Magic = %X | Indirizzo = %p\n",
               count, current->size, current->is_free, current->magic, (void*)current);
        current = current->next;
        count++;
    }
    printf("---------------------------\n\n");
}

static block_header_t *find_free_block(size_t size) {
    block_header_t *current = head;
    while (current) {
        // Controllo validità tramite MAGIC
        if (current->is_free && current->size >= size && current->magic == MAGIC) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void split_block(block_header_t *block, size_t size) {
    // Lo splitting avviene solo se c'è spazio per un nuovo header e almeno 16 byte di payload
    if (block->size >= size + HEADER_SIZE + 16) {
        block_header_t *new_block = (block_header_t *)((uint8_t *)block + HEADER_SIZE + size);
        new_block->size = block->size - size - HEADER_SIZE;
        new_block->is_free = 1;
        new_block->magic = MAGIC;
        new_block->next = block->next;
        
        block->size = size;
        block->next = new_block;
    }
}

static block_header_t *request_space(block_header_t *last, size_t size) {
    // FIX BUG 2: Protezione da Integer Overflow su allocazioni enormi
    if (size > SIZE_MAX - HEADER_SIZE) {
        return NULL;
    }
    
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
    block->magic = MAGIC;
    block->next = NULL;
    
    return block;
}

void *my_malloc(size_t size) {
    if (size == 0) return NULL;
    
    size = align16(size);
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
    
    return (void *)((uint8_t *)block + HEADER_SIZE);
}

static void coalesce() {
    block_header_t *current = head;
    while (current && current->next) {
        // FIX BUG 1: Calcoliamo dove DOVREBBE trovarsi il prossimo blocco fisico
        uint8_t *expected_next_addr = (uint8_t *)current + HEADER_SIZE + current->size;
        
        // Fonde i blocchi SOLO se sono contigui nello spazio fisico dell'heap
        if (current->is_free && current->next->is_free && (uint8_t *)current->next == expected_next_addr) {
            current->size += HEADER_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

void my_free(void *ptr) {
    if (!ptr) return;
    
    // Torniamo indietro per leggere l'header
    block_header_t *block = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    
    // FIX BUG 7: Protezione contro puntatori invalidi e Double-Free
    if (block->magic != MAGIC) {
        fprintf(stderr, "Errore: Tentativo di liberare un puntatore non valido!\n");
        return; 
    }
    if (block->is_free) {
        fprintf(stderr, "Errore: Rilevato Double-Free sullo stesso puntatore!\n");
        return;
    }
    
    block->is_free = 1;
    coalesce();
}

void *my_calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) return NULL;
    
    // FIX BUG 3: Protezione contro overflow della moltiplicazione in calloc
    if (size && nmemb > SIZE_MAX / size) {
        return NULL;
    }
    
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
    if (!ptr) return my_malloc(size);
    
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)((uint8_t *)ptr - HEADER_SIZE);
    
    // Rifiuta puntatori non validi
    if (block->magic != MAGIC) return NULL;
    
    size_t aligned_size = align16(size);
    
    if (block->size >= aligned_size) {
        // FIX BUG 5: Splitta il blocco e recupera memoria se lo stiamo riducendo
        split_block(block, aligned_size);
        return ptr;
    }
    
    void *new_ptr = my_malloc(size);
    if (!new_ptr) return NULL;
    
    // FIX BUG 4: Copia strettamente il minimo necessario, evitando lettura di "spazzatura"
    size_t copy_size = block->size;
    if (size < copy_size) {
        copy_size = size;
    }
    
    uint8_t *src = (uint8_t *)ptr;
    uint8_t *dest = (uint8_t *)new_ptr;
    for (size_t i = 0; i < copy_size; i++) {
        dest[i] = src[i];
    }
    
    my_free(ptr);
    return new_ptr;
}
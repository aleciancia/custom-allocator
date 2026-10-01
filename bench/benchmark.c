
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "my_allocator.h"

#ifndef BENCH
#error "Compile with -DBENCH to enable the allocator's instrumentation counters"
#endif

#define DEFAULT_BLOCKS 10000
#define MIN_SIZE 16
#define MAX_SIZE 4096

static double elapsed_ms(clock_t start, clock_t end) {
    return (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
}

int main(int argc, char **argv) {
    unsigned seed = (argc > 1) ? (unsigned)strtoul(argv[1], NULL, 10) : 42;
    int n = (argc > 2) ? atoi(argv[2]) : DEFAULT_BLOCKS;
    if (n <= 0) {
        fprintf(stderr, "Usage: %s [seed] [n_blocks > 0]\n", argv[0]);
        return 1;
    }

    void **ptrs = malloc(n * sizeof *ptrs);
    size_t *sizes = malloc(n * sizeof *sizes);
    int *order = malloc(n * sizeof *order);
    if (!ptrs || !sizes || !order) {
        fprintf(stderr, "Benchmark setup failed: out of memory\n");
        return 1;
    }


    srand(seed);
    size_t user_bytes = 0;
    for (int i = 0; i < n; i++) {
        sizes[i] = MIN_SIZE + (size_t)(rand() % (MAX_SIZE - MIN_SIZE + 1));
        user_bytes += sizes[i];
        order[i] = i;
    }
    for (int i = n - 1; i > 0; i--) {   /* Fisher-Yates shuffle */
        int j = rand() % (i + 1);
        int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
    }

   
    clock_t t0 = clock();
    for (int i = 0; i < n; i++) {
        ptrs[i] = my_malloc(sizes[i]);
        if (!ptrs[i]) {
            fprintf(stderr, "my_malloc(%zu) failed at i=%d\n", sizes[i], i);
            return 1;
        }
    }
    clock_t t1 = clock();


    int errors = 0;
    for (int i = 0; i < n; i++) {
        if ((uintptr_t)ptrs[i] % 16 != 0) errors++;
        memset(ptrs[i], (unsigned char)i, sizes[i]);
    }
    for (int i = 0; i < n; i++) {
        unsigned char *p = ptrs[i];
        for (size_t k = 0; k < sizes[i]; k++) {
            if (p[k] != (unsigned char)i) { errors++; break; }
        }
    }


    clock_t t2 = clock();
    for (int i = 0; i < n; i++) {
        my_free(ptrs[order[i]]);
    }
    clock_t t3 = clock();

    double malloc_ms = elapsed_ms(t0, t1);
    double free_ms = elapsed_ms(t2, t3);
    double total_ms = malloc_ms + free_ms;
    size_t overhead = sbrk_bytes_total - user_bytes;

    printf("=== custom-allocator benchmark (seed=%u, blocks=%d, sizes %d-%d B) ===\n",
           seed, n, MIN_SIZE, MAX_SIZE);
    printf("Correctness   : %s\n", errors ? "FAILED" : "OK (all blocks 16-byte aligned, no overlap)");
    printf("[1] Throughput: %d operations in %.3f ms (malloc %.3f ms, free %.3f ms)\n",
           2 * n, total_ms, malloc_ms, free_ms);
    printf("                %.2f us/operation\n", total_ms * 1000.0 / (2 * n));
    printf("[2] Coalescing: %lu merges (max possible: %d)\n", coalescing_count, n - 1);
    printf("[3] User bytes: %zu (%.2f MB net)\n", user_bytes, user_bytes / 1048576.0);
    printf("    sbrk bytes: %zu (%.2f MB gross)\n", sbrk_bytes_total, sbrk_bytes_total / 1048576.0);
    printf("    Overhead  : %zu bytes (%.2f%%)\n", overhead, 100.0 * overhead / user_bytes);

    free(ptrs);
    free(sizes);
    free(order);
    return errors ? 1 : 0;
}
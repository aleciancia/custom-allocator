#include <stdio.h>
#include "my_allocator.h"

int main() {
    printf("1. Alloco ptr1 (100 byte)\n");
    void *ptr1 = my_malloc(100);
    print_memory_map();

    printf("2. Alloco ptr2 (200 byte)\n");
    void *ptr2 = my_malloc(200);
    print_memory_map();

    printf("3. Alloco ptr3 (50 byte)\n");
    void *ptr3 = my_malloc(50);
    (void)ptr3; // Risolve il warning "unused variable"
    print_memory_map();

    printf("4. Libero ptr2 (Il blocco centrale diventa 'Free = 1')\n");
    my_free(ptr2);
    print_memory_map();

    printf("5. Libero ptr1 (Coalescing! ptr1 e ptr2 si fondono in un unico grande blocco)\n");
    my_free(ptr1);
    print_memory_map();

    printf("6. Alloco ptr4 (50 byte). (Splitting! Il grande blocco libero viene tagliato)\n");
    void *ptr4 = my_malloc(50);
    (void)ptr4; // Risolve il warning "unused variable"
    print_memory_map();

    return 0;
}
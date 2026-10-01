# Custom Memory Allocator

A custom memory allocator in C built from scratch. It replaces standard `malloc`, `free`, `calloc`, and `realloc` using `sbrk()`, and manages memory blocks via a linked list.

## Features
- **First-Fit Allocation:** Fast search for available free memory blocks.
- **Block Splitting:** Prevents internal fragmentation by cutting large free blocks into smaller, exact-sized chunks.
- **Coalescing:** Safely merges physically adjacent free blocks to prevent external fragmentation.
- **16-byte Alignment:** Ensures strict memory alignment for modern 64-bit architectures (`max_align_t`).
- **Security:** Implements "Magic Numbers" to detect memory corruption and prevent Double-Free vulnerabilities.

## How to run
The project includes a `Makefile` configured with Clang and AddressSanitizer for strict memory debugging.

```bash
make
./test_allocator
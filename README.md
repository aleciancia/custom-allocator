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
## Benchmark
make bench && ./benchmark [seed] [n_blocks]

Results (seed 42, 10,000 blocks of 16–4096 B, shuffled free order, Linux x86_64, -O2):
- Coalescing: 9,999 / 9,999 merges — the heap fully collapses into one free block
- Memory overhead: 1.92% (19.62 MB requested, 20.00 MB obtained via sbrk)
- Throughput: ~1.8 s for 20,000 operations. Both allocation and coalescing
  walk the full block list (O(n) per call), which is the main known limitation.

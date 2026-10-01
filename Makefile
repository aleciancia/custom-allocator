CC = clang
CFLAGS = -Wall -Wextra -g -I./include -fsanitize=address -Wno-deprecated-declarations

SRC = src/my_allocator.c tests/test.c
TARGET = test_allocator

# Benchmark: optimized build, no AddressSanitizer (it would distort timings
# and interfere with sbrk), instrumentation counters enabled via -DBENCH
BENCH_CFLAGS = -Wall -Wextra -O2 -I./include -DBENCH -Wno-deprecated-declarations
BENCH_SRC = src/my_allocator.c bench/benchmark.c
BENCH_TARGET = benchmark

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

bench: $(BENCH_TARGET)

$(BENCH_TARGET): $(BENCH_SRC) include/my_allocator.h
	$(CC) $(BENCH_CFLAGS) -o $(BENCH_TARGET) $(BENCH_SRC)

clean:
	rm -f $(TARGET) $(BENCH_TARGET)
	rm -rf *.dSYM

.PHONY: all bench clean
CC = clang
CFLAGS = -Wall -Wextra -g -I./include -fsanitize=address -Wno-deprecated-declarations

SRC = src/my_allocator.c tests/test.c
TARGET = test_allocator

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
	rm -rf *.dSYM
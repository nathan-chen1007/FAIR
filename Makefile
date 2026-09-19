CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

fair: src/*.c src/*.h
	$(CC) $(CFLAGS) -o fair src/*.c
test_window: $(filter-out src/main.c,$(wildcard src/*.c)) tests/test_window.c src/*.h
	$(CC) $(CFLAGS) -o test_window $(filter-out src/main.c,$(wildcard src/*.c)) tests/test_window.c -Isrc
test: test_window
	./test_window
clean:
	rm -f fair test_window

.PHONY: test clean

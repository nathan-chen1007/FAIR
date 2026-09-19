CC = gcc
FLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

fair: src/*.c src/*.h
	$(CC) $(CFLAGS) -o fair src/*.c
clean:
	rm -f fair


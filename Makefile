CC = gcc
FLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined

fair: src/main.c
	$(CC) $(CFLAGS) -o fair src/main.c

clean:
	rm -f fair


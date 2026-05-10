CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=address -g

minimake: main.c
	$(CC) $(CFLAGS) -o minimake main.c

clean:
	rm -f minimake

.PHONY: clean
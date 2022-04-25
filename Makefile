CC=gcc
CFLAGS= -std=gnu99 -pthread -Wall -Wextra -Werror -pedantic

all: proj2.c
	$(CC) $(CFLAGS) proj2.c -o proj2 -lrt

zip:
	zip proj2.zip proj2.c Makefile
clean:
	rm -rf *.zip *.out

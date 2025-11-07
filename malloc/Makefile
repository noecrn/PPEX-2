CC=gcc
CFLAGS=-std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fvisibility=hidden -fPIC
SRCS=malloc.c calloc.c realloc.c free.c
OBJS=$(SRCS:.c=.o)

.PHONY:library check clean

all: library

library: CPPFLAGS+=-D_DEFAULT_SOURCE
library: LDFLAGS+=-shared -Wl,--no-undefined
library:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/malloc.c -o malloc.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/calloc.c -o calloc.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/realloc.c -o realloc.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/free.c -o free.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/linker.c -o linker.o
	$(CC) -o libmalloc.so $(OBJS) linker.o $(LDFLAGS)

check: library
	./tests/test_suite.sh

test: library
	$(CC) -o test tests/test_basic.c $(OBJS) -lcriterion

clean:
	$(RM) libmalloc.so $(OBJS) malloc.tar linker.o test

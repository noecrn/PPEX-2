CC=gcc
CFLAGS=-std=c99 -pedantic -Werror -Wall -Wextra -Wvla -fvisibility=hidden -fPIC -g
SRCS=malloc.c calloc.c realloc.c
OBJS=$(SRCS:.c=.o)

.PHONY:library check clean debug

all: library

library: CPPFLAGS+=-D_DEFAULT_SOURCE
library: LDFLAGS+=-shared -Wl,--no-undefined
library:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/malloc.c -o malloc.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/calloc.c -o calloc.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -c src/realloc.c -o realloc.o
	$(CC) -o libmalloc.so $(OBJS) $(LDFLAGS)

debug:

check: library
	./tests/test_suite.sh

clean:
	$(RM) libmalloc.so $(OBJS) malloc.tar

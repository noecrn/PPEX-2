export CC=gcc
export CFLAGS=-std=c99 -Werror -Wall -Wextra -Wvla -pedantic

TARGET=httpd

.PHONY: all check clean

all: $(TARGET)

$(TARGET):
	$(MAKE) -C src

check:
	@echo "Lacer les tests"

clean:
	$(MAKE) -C src clean
	$(RM) $(TARGET)

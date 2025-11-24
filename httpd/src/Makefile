MODULES=config server daemon utils/string logger http

SRC=main.c
OBJS=$(SRC:.c=.o)

TARGET=../httpd

.PHONY: all $(MODULES) clean

# Compile main, sub-dir then link everything
all: $(MODULES) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(shell find $(MODULES) -name "*.o") -o $(TARGET)

# Compile every sub-dir
$(MODULES):
	$(MAKE) -C $@

# Compile main.c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Loop to clean every sub-dir
clean:
	$(RM) $(OBJS)
	$(foreach dir, $(MODULES), $(MAKE) -C $(dir) clean;)

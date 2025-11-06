#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

struct block *cast_block(void *ptr, size_t offset);
size_t align(size_t size);
void split_block(struct block *free_block, size_t needed);
void merge_block(struct block *a, struct block *b);
void *malloc(size_t size);
void free(void *ptr);

#endif /* MALLOC_H */

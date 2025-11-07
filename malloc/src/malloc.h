#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include "metadata.h"

extern struct block_list g_list;

struct block *cast_block(void *ptr, int offset, size_t flag);
size_t align(size_t size);
void split_block(struct block *free_block, size_t needed);
void merge_block(struct block *a, struct block *b);

void *my_malloc(size_t size);

#endif /* MALLOC_H */

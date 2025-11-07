#include <string.h>

#include "malloc.h"
#include "free.h"
#include "metadata.h"
#include <stdlib.h>

void *my_realloc(void *ptr, size_t size)
{
    if (!ptr)
    {
        return my_malloc(size);
    }

    if (size == 0)
    {
        my_free(ptr);
        return NULL;
    }

    // Get block address
    struct block *block = cast_block(ptr, sizeof(struct block), 0);
    size_t needed = align(size) + sizeof(struct block);

    // If no need to modifie the size
    if (needed == block->size)
    {
        return ptr;
    }

    // If the size is smaller
    if (needed < block->size)
    {
        split_block(block, needed);
        return ptr;
    }

    // If the next block is free and the both size are big enough
    struct block *next = block->next;
    if (next && next->status == 0 && (block->size + next->size >= needed))
    {
        merge_block(block, block->next);
        split_block(block, needed);
        return ptr;
    }

    // Get the minimum of current size and new size
    size_t current_size = block->size - sizeof(struct block);
    size_t new_size = (current_size < block->size) ? current_size : size;

    // If the size is bigger
    void *new_ptr = my_malloc(size);
    if (!new_ptr)
    {
        return NULL;
    }
    memcpy(new_ptr, ptr, new_size);
    my_free(ptr);

    return new_ptr;
}

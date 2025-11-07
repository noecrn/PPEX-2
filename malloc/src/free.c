#include "metadata.h"
#include "malloc.h"

#include <sys/mman.h>

// Merge two blocks together
void merge_block(struct block *a, struct block *b)
{
    // If b is the last element
    if (g_list.tail == b)
    {
        g_list.tail = a;
    }

    // Merge of two blocks
    a->next = b->next;
    if (b->next)
    {
        b->next->prev = a;
    }
    a->size += b->size;

    // Delete the merged block
    b->size = 0;
    b->next = NULL;
    b->prev = NULL;
    b->status = 0;
}

void my_free(void *ptr)
{
    // Invalid pointer
    if (!ptr)
    {
        return;
    }

    // Get the block address & mark it free
    struct block *block = cast_block(ptr, sizeof(struct block), 0);
    block->status = 0;

    // If the next exists and is free
    if (block->next && block->next->status == 0)
    {
        merge_block(block, block->next);
    }

    // If the previous exists and is free
    if (block->prev && block->prev->status == 0)
    {
        struct block *prev = block->prev;
        merge_block(prev, block);
        block = prev;
    }

    // The last block standing in the memory
    if (!block->next && !block->prev)
    {
        // If munmap succeed
        if (munmap(block, block->size) == 0)
        {
            g_list.head = NULL;
            g_list.tail = NULL;
        }
        // If munmap failed make shure block is in the list
        else
        {
            g_list.head = block;
            g_list.tail = block;
        }
    }
    else
    {
        // The last block of the queue
        if (!block->next)
        {
            g_list.tail = block;
        }

        // The first block of the queue
        if (!block->prev)
        {
            g_list.head = block;
        }
    }
}


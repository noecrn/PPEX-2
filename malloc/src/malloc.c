#include "metadata.h"

#include <stddef.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <stdint.h>
#include <err.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

// Global var for the list
struct block_list g_list = { NULL, NULL };

struct block *cast_block(void *ptr, size_t offset)
{
    return (struct block *)((char *)ptr - offset);
}

// First fit algorithm (can be optimised in Best fit algorithm)
static struct block *find_block(size_t size)
{
    struct block *cur = g_list.head;
    while (cur)
    {
        // If the block fit the size
        if (cur->size >= size && cur->status == 0)
        {
            return cur;
        }
        cur = cur->next;
    }

    // No block found
    return NULL;
}

size_t align(size_t size)
{
    size_t align = sizeof(long double);

    // Size is not a multiple of 8
    if (size % align != 0)
    {
        // Check overflow
        size_t res = 0;
        if (__builtin_add_overflow(size, align - 1, &res)
            || __builtin_mul_overflow(res / align, align, &res))
        {
            // Overflow detected
            return 0;
        }
        return res;
    }

    // Size is already a multiple of 8
    return size;
}

static size_t expand_memory(size_t needed)
{
    // Compute the number of pages
    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t nb_page = (needed + page_size - 1) / page_size;

    // At least one page
    if (nb_page == 0)
    {
        nb_page = 1;
    }

    // Allocate the number of pages
    size_t length = nb_page * page_size;
    struct block *free_block = mmap(NULL, length, PROT_READ | PROT_WRITE,
                                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (free_block == MAP_FAILED)
    {
        return 1;
    }

    // Init free_block
    free_block->size = length;
    free_block->next = NULL;
    free_block->prev = NULL;
    free_block->status = 0;

    // If it's the first ever allocation
    if (!g_list.head || !g_list.tail)
    {
        g_list.head = free_block;
        g_list.tail = free_block;
    }
    else
    {
        // Add free block at the end of global list
        g_list.tail->next = free_block;
        free_block->prev = g_list.tail;
        g_list.tail = free_block;
    }

    return 0;
}

void split_block(struct block *free_block, size_t needed)
{
    // If unused space cannot contain a new block
    size_t unused_space = free_block->size - needed;
    size_t min_block_size = sizeof(struct block) + align(1);
    if (unused_space < min_block_size)
    {
        return;
    }

    // Init new block address
    struct block *new_block = cast_block(free_block, needed);
    new_block->size = unused_space;
    new_block->status = 0;

    // Update chain list & size
    new_block->next = free_block->next;
    new_block->prev = free_block;
    if (new_block->next)
    {
        new_block->next->prev = new_block;
    }
    free_block->next = new_block;
    free_block->size = needed;

    // Update queue of the list
    if (g_list.tail == free_block)
    {
        g_list.tail = new_block;
    }
}

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    // Memory cannot be allocated
    if (size == 0)
    {
        return NULL;
    }

    // Align size on long double
    size_t needed = align(size) + sizeof(struct block);

    // Find a free block that fit the size
    struct block *free_block = find_block(needed);

    // No free block big enough was found
    if (!free_block && expand_memory(needed) == 1)
    {
        return NULL;
    }
    // Free block was found
    else
    {
        split_block(free_block, needed);
    }

    // Update status & return pointer to the zone
    free_block->status = 1;
    return cast_block(free_block, sizeof(struct block));
}

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

__attribute__((visibility("default"))) void free(void *ptr)
{
    // Invalid pointer
    if (!ptr)
    {
        return;
    }

    // Get the block address & mark it free
    struct block *block = cast_block(ptr, sizeof(struct block));
    block->status = 0;

    // If the next exists and is free
    if (block->next && block->next->status == 0)
    {
        merge_block(block, block->next);
    }

    // If the previous exists and is free
    if (block->prev && block->prev->status == 0)
    {
        merge_block(block->prev, block);
        block = block->prev;
    }

    // The last block standing in the memory
    if (!block->next && !block->prev)
    {
        g_list.head = NULL;
        g_list.tail = NULL;
        munmap(block, block->size);
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

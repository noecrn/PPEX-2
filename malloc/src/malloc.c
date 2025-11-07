#include <assert.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "metadata.h"

// Global var for the list
struct block_list g_list = { NULL, NULL };

struct block *cast_block(void *ptr, int offset, size_t flag)
{
    if (flag == 0)
    {
        offset = -offset;
    }
    return (struct block *)((char *)ptr + offset);
}

// Best fit algorithm
static struct block *best_fit(size_t size)
{
    struct block *res = NULL;
    struct block *cur = g_list.head;

    while (cur)
    {
        // If free block found
        if (cur->status == 0 && cur->size >= size)
        {
            // If it's the first free block or
            // If the block fit the size and is smaller than the previous found
            if (!res || cur->size < res->size)
            {
                res = cur;
            }
        }
        cur = cur->next;
    }

    return res;
}

// Align size with multiple of sizeof(long double)
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

// Expand memory size to needed
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

// Split free_block in two
// One block of size needed and the other block of size left
void split_block(struct block *free_block, size_t needed)
{
    if (!free_block)
    {
        return;
    }

    // If unused space cannot contain a new block
    size_t unused_space = free_block->size - needed;
    size_t min_block_size = sizeof(struct block) + align(1);
    if (unused_space < min_block_size)
    {
        return;
    }

    // Init new block address
    struct block *new_block = cast_block(free_block, needed, 1);
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

void *my_malloc(size_t size)
{
    // Memory cannot be allocated
    if (size == 0)
    {
        return NULL;
    }

    // Align size on long double
    size_t needed = align(size) + sizeof(struct block);

    // Find a free block that fit the size
    struct block *free_block = best_fit(needed);

    // No free block big enough was found
    if (!free_block)
    {
        if (expand_memory(needed) == 1)
        {
            return NULL;
        }
        free_block = g_list.tail;
    }

    // Free block was found
    split_block(free_block, needed);

    // Update status & return pointer to the zone
    free_block->status = 1;
    return (void *)cast_block(free_block, sizeof(struct block), 1);
}


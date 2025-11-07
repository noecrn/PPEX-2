#include <errno.h>
#include <string.h>

#include "malloc.h"
#include "metadata.h"
#include <stdlib.h>

void *my_calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0)
    {
        return NULL;
    }

    // Check for overflow
    size_t total_size;
    if (__builtin_mul_overflow(nmemb, size, &total_size))
    {
        errno = ENOMEM;
        return NULL;
    }

    // Allocate memory
    void *block = my_malloc(total_size);
    if (!block)
    {
        return NULL;
    }

    // Set memory to 0
    memset(block, 0, total_size);

    return block;
}

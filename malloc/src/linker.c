#include "malloc.h"
#include "free.h"
#include "calloc.h"
#include "realloc.h"

#include <stddef.h>

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    return my_malloc(size);
}

__attribute__((visibility("default"))) void free(void *ptr)
{
    my_free(ptr);
}

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
    return my_calloc(nmemb, size);
}

__attribute__((visibility("default"))) void *realloc(void *ptr, size_t size)
{
    return my_realloc(ptr, size);
}

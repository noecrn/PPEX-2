#ifndef METADATA_H
#define METADATA_H

#include <stddef.h>

struct block
{
    size_t status;
    size_t size;
    struct block *next;
    struct block *prev;
};

struct block_list
{
    struct block *head;
    struct block *tail;
};

#endif /* METADATA_H */

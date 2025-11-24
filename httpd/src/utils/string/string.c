#include "string.h"

#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct string *string_create(const char *str, size_t size)
{
    struct string *new_string = malloc(sizeof(struct string));
    if (!new_string)
    {
        err(EXIT_FAILURE, "Malloc failed");
    }

    new_string->data = malloc(size);
    if (!new_string->data)
    {
        err(EXIT_FAILURE, "Malloc failed");
    }

    memcpy(new_string->data, str, size);
    new_string->size = size;

    return new_string;
}

int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    if (!str1 || !str1->data || !str2)
    {
        return -1;
    }

    return memcmp(str1->data, str2, n);
}

void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    if (size == 0)
    {
        return;
    }

    size_t old_size = str->size;
    size_t new_size = str->size + size;

    char *new_data = realloc(str->data, new_size);
    if (!new_data)
    {
        err(EXIT_FAILURE, "Realloc failed");
    }

    str->data = new_data;

    memcpy(new_data + old_size, to_concat, size);
    str->size = new_size;
}

void string_destroy(struct string *str)
{
    if (!str)
    {
        return;
    }

    free(str->data);
    free(str);
}

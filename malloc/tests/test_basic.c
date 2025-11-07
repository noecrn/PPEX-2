#include "../src/malloc.h"
#include "../src/free.h"
#include "../src/calloc.h"
#include "../src/realloc.h"

#include <string.h>
#include <criterion/criterion.h>

Test(malloc, simple_malloc)
{
    char *ptr = my_malloc(20);
    cr_assert_not_null(ptr, "Malloc failed");
    strcpy(ptr, "hello");
    cr_assert_str_eq(ptr, "hello", "Writing failed");
    my_free(ptr);
}

Test(malloc, malloc_zero)
{
    void *ptr = my_malloc(0);
    my_free(ptr);
    cr_assert(1, "Malloc 0 failed");
}

Test(free, free_null)
{
    my_free(NULL);
    cr_assert(1, "Free failed");
}

Test(calloc, simple_calloc)
{
    size_t count = 20;
    size_t size = sizeof(char);
    char *ptr = my_calloc(count, size);
    cr_assert_not_null(ptr, "Calloc failed");
    char expected[20] = {0};
    int res = memcmp(ptr, expected, count * size);
    cr_assert_eq(res, 0, "Calloc failed");
    my_free(ptr);
}

Test(realloc, realloc_malloc)
{
    char *ptr = my_realloc(NULL, 20);
    cr_assert_not_null(ptr, "Realloc failed");
    strcpy(ptr, "hello");
    cr_assert_str_eq(ptr, "hello", "Writing failed");
    my_free(ptr);
}

Test(realloc, realloc_free)
{
    void *ptr = my_malloc(20);
    void *result_ptr = my_realloc(ptr, 0);
    cr_assert(1, "Realloc failed");
}

Test(realloc, realloc_simple)
{
    char *ptr = my_malloc(20);
    strcpy(ptr, "This is a test");
    char *ptr_new = my_realloc(ptr, 40);
    cr_assert_not_null(ptr_new, "Realloc failed");
    cr_assert_str_eq(ptr_new, "This is a test", "Writing failed");
    my_free(ptr_new);
}

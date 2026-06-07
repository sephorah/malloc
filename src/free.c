#include "my_malloc.h"

extern size_t *heap_start;

void update_free_list(size_t *ptr)
{
    size_t *ptr_prev = NULL;
    size_t *old_block_prev = NULL;
    // size_t *old_block_next = NULL;

    if (heap_start == NULL)
    {
        return;
    }
    ptr_prev = (size_t *)(ptr + 1);
    if (*heap_start != 0)
    {
        *ptr = *heap_start;
        *ptr_prev = (size_t)heap_start;
        old_block_prev = (size_t *)(*heap_start + 8);
        // old_block_next = (size_t *)(*heap_start);
        *old_block_prev = (size_t)ptr;
    }
    else
    {
        *ptr = 0;
        *ptr_prev = 0;
    }
    *heap_start = (size_t)ptr;
}

void free(void *ptr)
{
    header_t *header_address = NULL;

    if (ptr == NULL)
    {
        return;
    }
    header_address = ptr - HEADER_SIZE;
    if (!is_allocated(header_address))
    {
        return;
    }
    fprintf(stderr, "[freed block] %p (%ld) : %ld bytes\n", header_address, (size_t)header_address, get_size(header_address));
    *header_address = CLEAR_BIT(*header_address);
    update_free_list(ptr);
}

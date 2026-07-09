#include "my_malloc.h"

extern size_t *heap_start;

static void add_block_free_list(size_t *ptr)
{
    size_t *ptr_prev = NULL;
    size_t *old_block_prev = NULL;

    if (heap_start == NULL)
    {
        return;
    }
    ptr_prev = (size_t *)(ptr + 1);
    if (*heap_start != 0)
    {
        *ptr = *heap_start;
        *ptr_prev = (size_t)heap_start;
        old_block_prev = (size_t *)(*heap_start + sizeof(size_t));
        *old_block_prev = (size_t)ptr;
    }
    else
    {
        *ptr = 0;
        *ptr_prev = (size_t)heap_start;
    }
    *heap_start = (size_t)ptr;
}

void free(void *ptr)
{
    boundary_tag_t *header_address = NULL;

    if (ptr == NULL)
    {
        return;
    }
    header_address = ptr - BOUNDARY_TAG_SIZE;
    if (!is_allocated(header_address))
    {
        return;
    }
    add_block_free_list(ptr);
    merge_free_blocks(ptr);
}

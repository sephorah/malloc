#include "my_malloc.h"
#include <stdlib.h>
extern size_t *heap_start;

// void check_boundary_tags(boundary_tag_t *header, boundary_tag_t *footer)
// {
//     return *header == *footer;
// }

void check_valid_list()
{
    size_t *block_tmp = NULL;
    size_t *block_tmp_next = NULL;

    if (*heap_start == 0)
    {
        return;
    }
    block_tmp = (size_t *)(*heap_start);
    while (block_tmp != NULL)
    {
        block_tmp_next = (size_t *)((size_t)block_tmp);
        block_tmp = (size_t *)(*block_tmp_next);
    }
}

void update_next_neighbor(size_t *block_header)
{
    size_t *current_next_block = (size_t *)*((size_t *)((size_t)block_header + HEADER_SIZE));
    size_t *new_next_block = NULL;

    if ((size_t)current_next_block == 0)
    {
        return;
    }
    new_next_block = (size_t *)*current_next_block;
    *current_next_block = *new_next_block;
}

void check_next_block(size_t *current_block_header, size_t *current_block_footer)
{
    boundary_tag_t *next_block_header = (size_t *)*(size_t *)((size_t)current_block_header + HEADER_SIZE);
    boundary_tag_t *next_block_footer = NULL;

    if ((size_t)next_block_header != ((size_t)current_block_footer + HEADER_SIZE))
    {
        return;
    }
    next_block_footer = (boundary_tag_t *)((size_t)next_block_header + get_size(next_block_header) - HEADER_SIZE);
    *(size_t *)(next_block_header + HEADER_SIZE) = 0;
    *(size_t *)(next_block_header + HEADER_SIZE + 8) = 0;
    *next_block_footer = get_size(next_block_header) + get_size(current_block_header);
    *current_block_header = *next_block_footer;
    update_next_neighbor(current_block_header);
    *next_block_header = 0;
    *current_block_footer = 0;
}

size_t *merge_free_blocks(size_t *current_block)
{
    boundary_tag_t *prev_block_footer = NULL;
    boundary_tag_t *prev_block_header = NULL;
    boundary_tag_t *current_block_header = NULL;
    boundary_tag_t *current_block_footer = NULL;
    size_t *prev_block = (size_t *)*(current_block + 1);
    size_t *next_block = (size_t *)*current_block;

    if (prev_block != 0)
    {
        if ((size_t)prev_block + get_size((size_t *)(*prev_block - HEADER_SIZE)) != (size_t)current_block - HEADER_SIZE)
        {
            return current_block;
        }
        prev_block_footer = (boundary_tag_t *)((size_t)current_block - HEADER_SIZE * 2);
        prev_block_header = (boundary_tag_t *)((size_t)prev_block_footer - get_size(prev_block_footer) + HEADER_SIZE);
        current_block_header = (boundary_tag_t *)((size_t)current_block - HEADER_SIZE);
        current_block_footer = (boundary_tag_t *)((size_t)current_block + get_size(current_block_header) - HEADER_SIZE * 2);
        *current_block_footer = get_size(prev_block_footer) + get_size(current_block_header);
        *prev_block_header = *current_block_footer;
        *current_block = 0;
        *(current_block + 1) = 0;
        current_block = (size_t *)((size_t)prev_block_header + HEADER_SIZE);
        current_block_header = prev_block_header;
        update_next_neighbor(current_block_header);
    }
    if (next_block != 0)
    {
        check_next_block(current_block_header, current_block_footer);
    }
    return current_block;
}

void add_block_free_list(size_t *ptr)
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
        old_block_prev = (size_t *)(*heap_start + 8);
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
    header_address = ptr - HEADER_SIZE;
    if (!is_allocated(header_address))
    {
        return;
    }
    *header_address = CLEAR_BIT(*header_address);
    add_block_free_list(ptr);
    merge_free_blocks(ptr);
}

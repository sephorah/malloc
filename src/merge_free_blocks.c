#include "my_malloc.h"

static void remove_merged_block_free_list(size_t *block_payload)
{
    size_t *next_block = (size_t *)*block_payload;
    size_t *prev_block = (size_t *)block_payload[1];

    if (next_block != NULL)
    {
        next_block[1] = (size_t)prev_block;
    }
    if (prev_block != NULL)
    {
        *prev_block = *block_payload;
    }
}

static void merge_next_block(size_t *current_block_header, size_t *current_block_footer)
{
    boundary_tag_t *next_block_header = (size_t *)((size_t)current_block_footer + BOUNDARY_TAG_SIZE);
    boundary_tag_t *next_block_footer = NULL;
    size_t *current_block = (size_t *)((size_t)next_block_header + BOUNDARY_TAG_SIZE);
    size_t next_block_size = 0;

    if (is_allocated(next_block_header))
    {
        return;
    }
    next_block_size = get_size(next_block_header);
    next_block_footer = get_footer(next_block_header, next_block_size);
    // next_block_footer = (boundary_tag_t *)((size_t)next_block_header + get_size(next_block_header) - BOUNDARY_TAG_SIZE); // footer helper function done
    *next_block_footer = get_size(next_block_header) + get_size(current_block_header);
    *current_block_header = *next_block_footer;
    remove_merged_block_free_list(current_block);
}

static boundary_tag_t *merge_prev_block(size_t *current_block, boundary_tag_t *current_block_header, boundary_tag_t *current_block_footer)
{
    boundary_tag_t *prev_block_footer = (boundary_tag_t *)((size_t)current_block - BOUNDARY_TAG_SIZE * 2); // footer helper function done
    boundary_tag_t *prev_block_header = NULL;

    if (prev_block_footer == 0 || is_allocated(prev_block_footer))
    {
        return current_block_header;
    }
    prev_block_header = (boundary_tag_t *)((size_t)prev_block_footer - get_size(prev_block_footer) + BOUNDARY_TAG_SIZE);
    *current_block_footer = get_size(prev_block_footer) + get_size(current_block_header);
    *prev_block_header = *current_block_footer;
    remove_merged_block_free_list(current_block);
    return prev_block_header;
}

size_t *merge_free_blocks(size_t *current_block)
{
    boundary_tag_t *current_block_header = get_header(current_block); // header helper done function ?
    size_t current_block_size = get_size(current_block_header);
    boundary_tag_t *current_block_footer = get_footer(current_block_header, current_block_size); // footer helper function
    // boundary_tag_t *current_block_footer = (boundary_tag_t *)((size_t)current_block + get_size(current_block_header) - BOUNDARY_TAG_SIZE * 2); // footer helper function

    mark_boundary_tag_free(current_block_header);
    mark_boundary_tag_free(current_block_footer);
    // *current_block_header = CLEAR_BIT(*current_block_header); // mark free helper function
    // *current_block_footer = CLEAR_BIT(*current_block_footer); // mark free helper function

    current_block_header = merge_prev_block(current_block, current_block_header, current_block_footer);
    merge_next_block(current_block_header, current_block_footer);
    return current_block;
}
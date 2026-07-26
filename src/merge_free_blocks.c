#include "my_malloc.h"

void remove_block_free_list(char *block_payload)
{
    size_t *next_block = (size_t *)*(size_t *)block_payload;
    size_t *prev_block = (size_t *)((size_t *)block_payload)[1];

    if (next_block != NULL && is_address_valid((size_t)next_block)) {
        next_block[1] = (size_t)prev_block;
    }
    if (prev_block != NULL && is_address_valid((size_t)prev_block)) {
        *prev_block = *(size_t *)block_payload;
    }
}

static boundary_tag_t *merge_next_block(boundary_tag_t *current_block_header, boundary_tag_t *current_block_footer)
{
    boundary_tag_t *next_block_header = (boundary_tag_t *)((boundary_tag_t)current_block_footer + BOUNDARY_TAG_SIZE);
    boundary_tag_t *next_block_footer = NULL;
    char *next_block_payload = (char *)((boundary_tag_t)next_block_header + BOUNDARY_TAG_SIZE);
    size_t next_block_size = 0;

    if (is_allocated(*next_block_header)) {
        return current_block_footer;
    }
    next_block_size = get_size(*next_block_header);
    next_block_footer = get_footer(next_block_header, next_block_size);
    *next_block_footer = get_size(*next_block_header) + get_size(*current_block_header);
    *current_block_header = *next_block_footer;
    remove_block_free_list(next_block_payload);
    return next_block_footer;
}

static boundary_tag_t *merge_prev_block(char *current_block_payload, boundary_tag_t *current_block_header, boundary_tag_t *current_block_footer)
{
    boundary_tag_t *prev_block_footer = (boundary_tag_t *)(current_block_payload - BOUNDARY_TAG_SIZE * 2);
    boundary_tag_t *prev_block_header = NULL;

    if (prev_block_footer == 0 || is_allocated(*prev_block_footer)) {
        return current_block_header;
    }
    prev_block_header = (boundary_tag_t *)((boundary_tag_t)prev_block_footer - get_size(*prev_block_footer) + BOUNDARY_TAG_SIZE);
    *current_block_footer = get_size(*prev_block_footer) + get_size(*current_block_header);
    *prev_block_header = *current_block_footer;
    remove_block_free_list(current_block_payload);
    return prev_block_header;
}

char *merge_free_blocks(char *current_block_payload)
{
    boundary_tag_t *current_block_header = get_header(current_block_payload);
    size_t current_block_size = get_size(*current_block_header);
    boundary_tag_t *current_block_footer = get_footer(current_block_header, current_block_size);

    current_block_header = merge_prev_block(current_block_payload, current_block_header, current_block_footer);
    mark_boundary_tag_free(current_block_header);
    current_block_footer = merge_next_block(current_block_header, current_block_footer);
    mark_boundary_tag_free(current_block_footer);
    return current_block_payload;
}

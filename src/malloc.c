#include "my_malloc.h"

extern size_t *heap_start;

void handle_free_block_found(boundary_tag_t *block_tmp_header, size_t *block_tmp_next, size_t *block_tmp, size_t size_tmp)
{
    boundary_tag_t *block_tmp_footer = (boundary_tag_t *)((size_t)block_tmp + size_tmp - BOUNDARY_TAG_SIZE * 2);
    size_t *block_tmp_prev = (size_t *)(block_tmp + 1);

    *(boundary_tag_t *)block_tmp_header = SET_BIT(block_tmp_header, 0);
    *(boundary_tag_t *)block_tmp_footer = SET_BIT(block_tmp_footer, 0);
    *(size_t *)(*block_tmp_prev) = *block_tmp_next;
    if (*block_tmp_next != 0)
    {
        *(size_t *)(*block_tmp_next + sizeof(size_t)) = *block_tmp_prev;
    }
}

static void *traverse_free_list(size_t size)
{
    size_t *block_tmp = (size_t *)(*heap_start);
    boundary_tag_t *block_tmp_header = NULL;
    size_t *block_tmp_next = NULL;
    size_t size_tmp = 0;

    while (block_tmp != NULL)
    {
        block_tmp_header = (boundary_tag_t *)((size_t)block_tmp - BOUNDARY_TAG_SIZE);
        size_tmp = get_size(block_tmp_header);
        block_tmp_next = (size_t *)((size_t)block_tmp);
        if (size_tmp >= size)
        {
            handle_free_block_found(block_tmp_header, block_tmp_next, block_tmp, size_tmp);
            return block_tmp;
        }
        block_tmp = (size_t *)(*block_tmp_next);
    }
    return NULL;
}

static void *find_free_block(size_t size)
{
    if (*heap_start == 0)
    {
        return NULL;
    }
    return traverse_free_list(size);
}

static void *add_free_block(size_t payload_size)
{
    boundary_tag_t *new_block_header_address = (boundary_tag_t *)((size_t)get_current_break() - BOUNDARY_TAG_SIZE);
    void *new_block_payload = NULL;
    size_t block_size = 0;
    size_t optional_padding_size = 0;
    void *optional_padding = NULL;

    if (new_block_header_address != NULL)
    {
        block_size = get_aligned_block_size(payload_size);
        *new_block_header_address = block_size;
        *new_block_header_address = SET_BIT(new_block_header_address, 0);
        new_block_payload = allocate_block(block_size - BOUNDARY_TAG_SIZE);
        optional_padding_size = block_size - (BOUNDARY_TAG_SIZE * 2 + payload_size);
        optional_padding = add_optional_padding(payload_size, new_block_payload);
        init_boundary_tag(optional_padding + optional_padding_size, block_size);
        init_epilogue();
        return new_block_payload;
    }
    return NULL;
}

void *malloc(size_t size)
{
    void *free_block = NULL;
    size_t block_size = 0;

    init_heap();
    if (heap_start == NULL)
    {
        return NULL;
    }
    block_size = get_aligned_block_size(size);
    free_block = find_free_block(block_size);
    if (free_block == NULL)
    {
        return add_free_block(size);
    }
    return free_block;
}

#include "my_malloc.h"

extern size_t *heap_start;

size_t get_aligned_block_size(size_t payload_size)
{
    size_t total_size = HEADER_SIZE * 2 + payload_size;
    size_t div = total_size / ALIGNMENT_REQUIREMENT;
    size_t res = div * ALIGNMENT_REQUIREMENT;

    if (total_size % ALIGNMENT_REQUIREMENT != 0)
    {
        res += ALIGNMENT_REQUIREMENT;
    }
    return res;
}

size_t *add_optional_padding(size_t payload_size, void *start)
{
    return (size_t *)((size_t)start + payload_size);
}

void *find_free_block(size_t size)
{
    size_t *block_tmp = NULL;
    boundary_tag_t *block_tmp_header = NULL;
    size_t *block_tmp_prev = NULL;
    size_t *block_tmp_next = NULL;
    size_t size_tmp = 0;

    if (*heap_start == 0)
    {
        return NULL;
    }
    block_tmp = (size_t *)(*heap_start);
    block_tmp_header = (boundary_tag_t *)((size_t)block_tmp - HEADER_SIZE);
    size_tmp = get_size(block_tmp_header);

    while (block_tmp != NULL)
    {
        block_tmp_header = (boundary_tag_t *)((size_t)block_tmp - HEADER_SIZE);
        size_tmp = get_size(block_tmp_header);
        block_tmp_prev = (size_t *)(block_tmp + 1);
        block_tmp_next = (size_t *)((size_t)block_tmp);

        if (size_tmp >= size)
        {
            block_tmp_header = (boundary_tag_t *)((size_t)block_tmp - HEADER_SIZE);
            *(boundary_tag_t *)block_tmp_header = SET_BIT(block_tmp_header, 0);
            *(size_t *)(*block_tmp_prev) = *block_tmp_next;

            if (*block_tmp_next != 0)
            {
                *(size_t *)(*block_tmp_next + 8) = *block_tmp_prev;
            }
            return block_tmp;
        }
        block_tmp = (size_t *)(*block_tmp_next);
    }
    return NULL;
}

boundary_tag_t *init_boundary_tag(boundary_tag_t *tag_start, size_t size)
{
    *tag_start = size;
    *tag_start = SET_BIT(tag_start, 0);
    return tag_start;
}

void *allocate_block(size_t size)
{
    void *block_start = sbrk(size);

    if (block_start == (void *)-1)
    {
        perror("Error when allocating header");
        return NULL;
    }
    return block_start;
}

void *get_payload_start(void *start)
{
    return start + HEADER_SIZE;
}

void *init_epilogue()
{
    void *block_start = allocate_block(HEADER_SIZE);

    if (block_start == NULL)
    {
        return NULL;
    }
    return init_boundary_tag(block_start, HEADER_SIZE);
}

void *init_block(size_t payload_size)
{
    size_t block_size = get_aligned_block_size(payload_size);
    size_t optional_padding_size = block_size - (HEADER_SIZE * 2 + payload_size);
    boundary_tag_t *block_start = allocate_block(block_size);
    boundary_tag_t *header_start = NULL;
    boundary_tag_t *footer_start = NULL;
    void *payload_start = NULL;
    void *optional_padding = NULL;

    if (block_start == NULL)
    {
        return NULL;
    }
    header_start = init_boundary_tag(block_start, block_size);
    payload_start = get_payload_start(header_start);
    if (payload_start == NULL)
    {
        return NULL;
    }
    optional_padding = add_optional_padding(payload_size, payload_start);
    if (optional_padding == NULL)
    {
        return NULL;
    }
    footer_start = init_boundary_tag(optional_padding + optional_padding_size, block_size);
    if (footer_start == NULL)
    {
        return NULL;
    }
    return payload_start;
}

void *add_free_block(size_t payload_size)
{
    boundary_tag_t *new_block_header_address = (boundary_tag_t *)((size_t)get_current_break() - HEADER_SIZE);
    void *new_block_payload = NULL;
    size_t block_size = 0;
    size_t optional_padding_size = 0;
    void *optional_padding = NULL;

    if (new_block_header_address != NULL)
    {
        block_size = get_aligned_block_size(payload_size);
        *new_block_header_address = block_size;
        *new_block_header_address = SET_BIT(new_block_header_address, 0);
        new_block_payload = allocate_block(block_size - HEADER_SIZE);
        optional_padding_size = block_size - (HEADER_SIZE * 2 + payload_size);
        optional_padding = add_optional_padding(payload_size, new_block_payload);
        init_boundary_tag(optional_padding + optional_padding_size, block_size);
        init_epilogue();
        return new_block_payload;
    }
    return NULL;
}

void init_heap(void)
{
    if ((size_t *)heap_start != NULL)
    {
        return;
    }
    heap_start = init_block(8);
    init_epilogue();
}

void *malloc(size_t size)
{
    void *free_block = NULL;
    size_t block_size = 0;

    init_heap();
    if (heap_start == NULL)
    {
        perror("Error when initializing heap");
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

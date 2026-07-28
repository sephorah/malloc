#include "my_malloc.h"

static size_t get_valid_payload_size(size_t original_size)
{
    return (original_size < 16) ? 16 : original_size;
}

size_t get_aligned_block_size(size_t original_payload_size)
{
    size_t payload_size = get_valid_payload_size(original_payload_size);
    size_t total_size = 0;
    size_t div = 0;
    size_t res = 0;

    if (__builtin_add_overflow(BOUNDARY_TAG_SIZE * 2, payload_size, &total_size)) {
        return 0;
    }
    div = total_size / ALIGNMENT_REQUIREMENT;
    res = div * ALIGNMENT_REQUIREMENT;
    if (total_size % ALIGNMENT_REQUIREMENT != 0) {
        res += ALIGNMENT_REQUIREMENT;
    }
    return res;
}

char *add_optional_padding(size_t payload_size, void *start)
{
    return (char *)(start + payload_size);
}

boundary_tag_t *init_boundary_tag(boundary_tag_t *tag, size_t size)
{
    *tag = size;
    mark_boundary_tag_allocated(tag);
    return tag;
}

void *allocate_block(size_t size)
{
    void *block_start = sbrk(size);

    if (block_start == (void *)-1) {
        return NULL;
    }
    return block_start;
}

static void *init_block_content(boundary_tag_t *block_start, size_t block_size,
                                size_t optional_padding_size, size_t payload_size)
{
    boundary_tag_t *header_start = init_boundary_tag(block_start, block_size);
    void *payload_start = (void *)header_start + BOUNDARY_TAG_SIZE;
    void *optional_padding = NULL;

    optional_padding = add_optional_padding(payload_size, payload_start);
    init_boundary_tag(optional_padding + optional_padding_size, block_size);
    return payload_start;
}

void *init_block(size_t payload_size)
{
    size_t block_size = get_aligned_block_size(payload_size);
    size_t optional_padding_size = 0;
    boundary_tag_t *block_start = NULL;

    if (block_size == 0) {
        return NULL;
    }
    block_start = allocate_block(block_size);
    optional_padding_size = block_size - (BOUNDARY_TAG_SIZE * 2 + payload_size);
    if (block_start == NULL) {
        return NULL;
    }
    return init_block_content(block_start, block_size, optional_padding_size, payload_size);
}

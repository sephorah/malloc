#include "my_malloc.h"

int is_allocated(boundary_tag_t *header)
{
    return CHECK_BIT(*header, 0);
}

size_t get_size(boundary_tag_t *header)
{
    return CLEAR_BIT(*header);
}

void *get_current_break(void)
{
    void *current_break = sbrk(0);

    if (current_break == (void *)-1)
    {
        return NULL;
    }
    return current_break;
}

boundary_tag_t *get_header(size_t *block)
{
    return (boundary_tag_t *)((size_t)block - BOUNDARY_TAG_SIZE);
}

boundary_tag_t *get_footer(size_t *header, size_t size)
{
    return (boundary_tag_t *)((size_t)header + size - BOUNDARY_TAG_SIZE);
}

void mark_boundary_tag_allocated(boundary_tag_t *tag)
{
    *tag = SET_BIT(*tag, 0);
}

void mark_boundary_tag_free(boundary_tag_t *tag)
{
    *tag = CLEAR_BIT(*tag);
}
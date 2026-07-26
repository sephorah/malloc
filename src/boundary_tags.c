#include "my_malloc.h"

boundary_tag_t *get_header(char *block)
{
    return (boundary_tag_t *)(block - BOUNDARY_TAG_SIZE);
}

boundary_tag_t *get_footer(boundary_tag_t *header, size_t size)
{
    return (boundary_tag_t *)((boundary_tag_t)header + size - BOUNDARY_TAG_SIZE);
}

void mark_boundary_tag_allocated(boundary_tag_t *tag)
{
    *tag = SET_BIT(*tag, 0);
}

void mark_boundary_tag_free(boundary_tag_t *tag)
{
    *tag = CLEAR_BIT(*tag);
}

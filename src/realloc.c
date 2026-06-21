#include "my_malloc.h"

bool check_available_space(void *ptr, size_t size)
{
    boundary_tag_t *header = ptr - HEADER_SIZE;
    size_t block_size = get_size(header);

    if (block_size >= size)
    {
        return true;
    }
    // If we can, we should try to recycle the now-freed memory into another freed block.
    return false;
}

void *realloc(void *ptr, size_t size)
{
    void *new_block = NULL;

    if (ptr == NULL)
    {
        return malloc(size);
    }
    if (size == 0 && ptr != NULL)
    {
        free(ptr);
        return NULL;
    }
    if (check_available_space(ptr, size))
    {
        return ptr;
    }
    new_block = malloc(size);
    memcpy(new_block, ptr, size);
    free(ptr);
    return new_block;
}

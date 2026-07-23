#include "my_malloc.h"

static bool check_available_space(void *ptr, size_t size)
{
    boundary_tag_t *header = get_header(ptr);                       // header helper done function
    size_t payload_size = get_size(header) - BOUNDARY_TAG_SIZE * 2; // get payload size helper

    if (payload_size >= size)
    {
        return true;
    }
    return false;
}

static size_t get_old_block_payload_size(size_t *ptr, size_t size)
{
    boundary_tag_t *header = get_header(ptr);                           // header helper done function
    size_t old_payload_size = get_size(header) - BOUNDARY_TAG_SIZE * 2; // get payload size helper

    if (size < old_payload_size)
    {
        return size;
    }
    else
    {
        return old_payload_size;
    }
}

static void *allocate_new_block(void *ptr, size_t size)
{
    void *new_block = malloc(size);
    size_t old_block_payload_size = 0;

    if (new_block == NULL)
    {
        return ptr;
    }
    old_block_payload_size = get_old_block_payload_size(ptr, size);
    memcpy(new_block, ptr, old_block_payload_size);
    free(ptr);
    return new_block;
}

void *realloc(void *ptr, size_t size)
{
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
    return allocate_new_block(ptr, size);
}

#include "my_malloc.h"
extern size_t *heap_start;

void add_block_free_list(size_t *ptr)
{
    size_t *ptr_prev = NULL;
    size_t *old_block_prev = NULL;

    if (heap_start == NULL) {
        return;
    }
    ptr_prev = (size_t *)(ptr + 1);
    if (*heap_start != 0) {
        *ptr = *heap_start;
        *ptr_prev = (size_t)heap_start;
        old_block_prev = (size_t *)(*heap_start + sizeof(size_t));
        *old_block_prev = (size_t)ptr;
    } else {
        *ptr = 0;
        *ptr_prev = (size_t)heap_start;
    }
    *heap_start = (size_t)ptr;
}

void free(void *ptr)
{
    boundary_tag_t *header_address = NULL;

    pthread_mutex_lock(&heap_start_mutex);
    if (ptr == NULL || heap_start == NULL) {
        pthread_mutex_unlock(&heap_start_mutex);
        return;
    }
    header_address = get_header(ptr);
    if (!is_allocated(*header_address)) {
        pthread_mutex_unlock(&heap_start_mutex);
        return;
    }
    add_block_free_list(ptr);
    merge_free_blocks(ptr);
    pthread_mutex_unlock(&heap_start_mutex);
}

#include "my_malloc.h"

extern char *heap_start;

void add_block_free_list(char *ptr)
{
    char *ptr_prev = NULL;
    size_t *old_block_prev = NULL;

    if (heap_start == NULL) {
        return;
    }
    ptr_prev = ptr + sizeof(void *);
    if (*(size_t *)heap_start != 0) {
        *(size_t *)ptr = *(size_t *)heap_start;
        *(size_t *)ptr_prev = (size_t)heap_start;
        old_block_prev = (size_t *)(*(size_t *)heap_start + sizeof(void *));
        *old_block_prev = (size_t)(size_t *)ptr;
    } else {
        *(size_t *)ptr = 0;
        *(size_t *)ptr_prev = (size_t)heap_start;
    }
    *(size_t *)heap_start = (size_t)ptr;
}

__attribute__((visibility("default"))) void free(void *ptr)
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

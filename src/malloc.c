#include "my_malloc.h"

extern char *heap_start;

static void handle_free_block_found(boundary_tag_t *block_header, char *block_payload, size_t size)
{
    remove_block_free_list(block_payload);
    handle_leftover_space(block_header, size);
}

char *get_next_element(char *block)
{
    size_t *next_element = NULL;

    if (block == NULL || !is_address_valid((size_t)(size_t *)block)) {
        return NULL;
    }
    next_element = (size_t *)*(size_t *)block;
    if (next_element == NULL || !is_address_valid((size_t)next_element)) {
        return NULL;
    }
    return (char *)next_element;
}

static void *traverse_free_list(size_t size)
{
    char *block_tmp = (char *)*(size_t *)heap_start;
    boundary_tag_t *block_tmp_header = NULL;
    size_t size_tmp = 0;

    while (block_tmp != NULL) {
        block_tmp_header = get_header(block_tmp);
        size_tmp = get_size(*block_tmp_header);
        if (size_tmp >= size) {
            handle_free_block_found(block_tmp_header, block_tmp, size);
            return block_tmp;
        }
        block_tmp = get_next_element(block_tmp);
    }
    return NULL;
}

static void *find_free_block(size_t size)
{
    if (*(size_t *)heap_start == 0) {
        return NULL;
    }
    return traverse_free_list(size);
}

static void *create_free_block(boundary_tag_t *new_block_header_address, size_t payload_size)
{
    size_t block_size = get_aligned_block_size(payload_size);
    size_t optional_padding_size = 0;
    void *new_block_payload = NULL;
    void *optional_padding = NULL;

    if (block_size == 0) {
        return NULL;
    }
    new_block_payload = allocate_block(block_size - BOUNDARY_TAG_SIZE);
    if (new_block_payload != NULL) {
        init_boundary_tag(new_block_header_address, block_size);
        optional_padding_size = block_size - (BOUNDARY_TAG_SIZE * 2 + payload_size);
        optional_padding = add_optional_padding(payload_size, new_block_payload);
        init_boundary_tag(optional_padding + optional_padding_size, block_size);
        init_epilogue();
        return new_block_payload;
    }
    return NULL;
}

static void *add_free_block(size_t payload_size)
{
    char *current_break = get_current_break();
    boundary_tag_t *new_block_header_address = get_header(current_break);

    if (new_block_header_address == NULL) {
        return NULL;
    }
    return create_free_block(new_block_header_address, payload_size);
}

static void *get_suitable_block(size_t block_size, size_t payload_size)
{
    size_t *free_block = find_free_block(block_size);

    if (free_block == NULL) {
        free_block = add_free_block(payload_size);
        pthread_mutex_unlock(&heap_start_mutex);
        return free_block;
    }
    pthread_mutex_unlock(&heap_start_mutex);
    return free_block;
}

__attribute__((visibility("default"))) void *malloc(size_t size)
{
    size_t block_size = 0;

    pthread_mutex_lock(&heap_start_mutex);
    init_heap();
    if (heap_start == NULL) {
        pthread_mutex_unlock(&heap_start_mutex);
        return NULL;
    }
    block_size = get_aligned_block_size(size);
    if (block_size == 0) {
        pthread_mutex_unlock(&heap_start_mutex);
        return NULL;
    }
    return get_suitable_block(block_size, size);
}

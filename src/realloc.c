#include "my_malloc.h"

static bool check_available_space(char *ptr, size_t size)
{
    boundary_tag_t *header = get_header(ptr);
    size_t payload_size = get_size(*header) - BOUNDARY_TAG_SIZE * 2;

    if (payload_size >= size) {
        return true;
    }
    return false;
}

static size_t get_old_block_payload_size(char *ptr, size_t size)
{
    boundary_tag_t *header = get_header(ptr);
    size_t old_payload_size = get_size(*header) - BOUNDARY_TAG_SIZE * 2;

    if (size < old_payload_size) {
        return size;
    } else {
        return old_payload_size;
    }
}

static void *allocate_new_block(char *ptr, size_t size)
{
    void *new_block = malloc(size);
    size_t old_block_payload_size = 0;

    if (new_block == NULL) {
        return ptr;
    }
    old_block_payload_size = get_old_block_payload_size(ptr, size);
    memcpy(new_block, ptr, old_block_payload_size);
    free(ptr);
    return new_block;
}

static void add_leftover_block(boundary_tag_t *header, size_t size)
{
    char *payload = (char *)((boundary_tag_t)header + BOUNDARY_TAG_SIZE);
    boundary_tag_t *footer = get_footer(header, size);

    *header = size;
    *footer = size;
    add_block_free_list(payload);
}

static void shrink_block(boundary_tag_t *header, size_t new_size, size_t leftover_size)
{
    boundary_tag_t *footer = get_footer(header, new_size);
    boundary_tag_t *leftover_block_header = (boundary_tag_t *)((boundary_tag_t)header + new_size);

    *header = new_size;
    *footer = new_size;
    mark_boundary_tag_allocated(header);
    mark_boundary_tag_allocated(footer);
    add_leftover_block(leftover_block_header, leftover_size);
}

void handle_leftover_space(boundary_tag_t *header, size_t new_size)
{
    size_t original_size = get_size(*header);
    long int leftover_size = original_size - new_size;
    boundary_tag_t *original_footer = get_footer(header, original_size);

    if (leftover_size >= MIN_BLOCK_SIZE) {
        shrink_block(header, new_size, leftover_size);
    } else {
        mark_boundary_tag_allocated(header);
        mark_boundary_tag_allocated(original_footer);
    }
}

void *realloc(void *ptr, size_t size)
{
    size_t block_size = 0;
    boundary_tag_t *header = NULL;

    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0 && ptr != NULL) {
        free(ptr);
        return NULL;
    }
    if (check_available_space(ptr, size)) {
        pthread_mutex_lock(&heap_start_mutex);
        block_size = get_aligned_block_size(size);
        header = get_header(ptr);
        handle_leftover_space(header, block_size);
        pthread_mutex_unlock(&heap_start_mutex);
        return ptr;
    }
    return allocate_new_block(ptr, size);
}

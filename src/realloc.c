#include "my_malloc.h"
// #include <stdio.h>

static bool check_available_space(size_t *ptr, size_t size)
{
    boundary_tag_t *header = get_header(ptr);
    size_t payload_size = get_size(*header) - BOUNDARY_TAG_SIZE * 2;

    if (payload_size >= size) {
        return true;
    }
    return false;
}

static size_t get_old_block_payload_size(size_t *ptr, size_t size)
{
    boundary_tag_t *header = get_header(ptr);
    size_t old_payload_size = get_size(*header) - BOUNDARY_TAG_SIZE * 2;

    if (size < old_payload_size) {
        return size;
    } else {
        return old_payload_size;
    }
}

static void *allocate_new_block(size_t *ptr, size_t size)
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

static void add_leftover_block(size_t *header, size_t size)
{
    size_t *payload = (size_t *)((size_t)header + BOUNDARY_TAG_SIZE);
    size_t *footer = get_footer(header, size);

    *header = size;
    *footer = size;
    // fprintf(stderr, "LEFTOVER HEADER %ld size %ld payload %ld\n", (size_t)header, size, (size_t)payload);
    add_block_free_list(payload);
    // fprintf(stderr, "Leftover header %ld footer %ld || content header %ld footer %ld`\n",  (size_t)header, (size_t)footer, *header, *footer);
}

static void shrink_block(size_t *header, size_t new_size, size_t leftover_size)
{
    size_t *footer = get_footer(header, new_size);
    size_t *leftover_block_header = (size_t *)((size_t)header + new_size);

    *header = new_size;
    *footer = new_size;
    mark_boundary_tag_allocated(header);
    mark_boundary_tag_allocated(footer);
    // if (leftover_size >= MIN_BLOCK_SIZE) { // was in handle leftover
    add_leftover_block(leftover_block_header, leftover_size);
    // fprintf(stderr, "SHRINK\n");
    // }
    // fprintf(stderr, "Shrunk header %ld footer %ld || content header %ld footer %ld\n",(size_t)header, (size_t)footer, *header, *footer);
}

void handle_leftover_space(size_t *header, size_t new_size)
{
    size_t original_size = get_size(*header);
    long int leftover_size = original_size - new_size;
    size_t *original_footer = get_footer(header, original_size);

    // fprintf(stderr, "HEEEEEEEEEEEEEY original %ld new %ld leftover %ld \n", original_size, new_size, leftover_size);
    if (leftover_size >= MIN_BLOCK_SIZE) {
        shrink_block(header, new_size, leftover_size);
    } else {
        mark_boundary_tag_allocated(header);
        mark_boundary_tag_allocated(original_footer);
    }
    // shrink_block(header, new_size, leftover_size);

    // previous
    // if (leftover_size >= MIN_BLOCK_SIZE) {
    //     shrink_block(header, new_size, leftover_size);
    //     // // fprintf(stderr, "SHRINK\n");
    // }
    //     size_t original_size = get_size(*header);
    // long int leftover_size = original_size - new_size;

    // // // fprintf(stderr, "HEEEEEEEEEEEEEY original %ld new %ld leftover %ld \n", original_size, new_size, leftover_size);
    // if (leftover_size >= MIN_BLOCK_SIZE) {
    //     // // fprintf(stderr, "SHRINK\n");
    //     shrink_block(header, new_size, leftover_size);
    // }
}

void *realloc(void *ptr, size_t size)
{
    size_t block_size = 0;
    size_t *header = NULL;

    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0 && ptr != NULL) {
        free(ptr);
        return NULL;
    }
    if (check_available_space(ptr, size)) {
        pthread_mutex_lock(&heap_start_mutex);
        // If we can, we should try to recycle the now-freed memory into another freed block.
        block_size = get_aligned_block_size(size);
        header = get_header(ptr);
        handle_leftover_space(header, block_size);
        pthread_mutex_unlock(&heap_start_mutex);
        return ptr;
    }
    return allocate_new_block(ptr, size);
}

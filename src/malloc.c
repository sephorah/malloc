#include "my_malloc.h"
// #include <stdio.h>
// #include <stdlib.h>
extern size_t *heap_start;

static void handle_free_block_found(boundary_tag_t *block_header, size_t *block_payload, size_t size)
{
    // boundary_tag_t *block_old_footer = get_footer(block_header, previous_size);
    // boundary_tag_t *block_footer = get_footer(block_header, size); // TO REMOVE
    // size_t *block_prev = block_payload + 1;

    // mark_boundary_tag_allocated(block_header); // TO REMOVE
    // mark_boundary_tag_allocated(block_footer); // TO REMOVE
    // mark_boundary_tag_allocated(block_old_footer);
    // *(size_t *)(*block_prev) = *block_payload; // change
    // if (*block_payload != 0) {
    //     *(size_t *)(*block_payload + sizeof(size_t)) = *block_prev;
    // }
    remove_block_free_list(block_payload);     // change
    handle_leftover_space(block_header, size); // change
}

size_t *get_next_element(size_t *block)
{
    size_t *block_next = (size_t *)((size_t)block);
    size_t *next_element = NULL;

    if (block == NULL || !is_address_valid(block) || block_next == NULL || !is_address_valid(block_next)) {
        return NULL;
    }
    next_element = (size_t *)(*block_next);
    if (next_element == NULL || !is_address_valid(next_element)) {
        return NULL;
    }
    return next_element;
}

static void *traverse_free_list(size_t size)
{
    size_t *block_tmp = (size_t *)(*heap_start);
    boundary_tag_t *block_tmp_header = NULL;
    // size_t *block_tmp_next = NULL;
    size_t size_tmp = 0;
    // // fprintf(stderr, "Hey list malloc %ld\n", size);

    while (block_tmp != NULL) {
        block_tmp_header = get_header(block_tmp);
        size_tmp = get_size(*block_tmp_header);
        // block_tmp_next = (size_t *)((size_t)block_tmp);
        // fprintf(stderr, "In traverse free list header %ld size %ld\n", (size_t)block_tmp_header, size_tmp);
        if (size_tmp >= size) {
            handle_free_block_found(block_tmp_header, block_tmp, size); // change to size for splitting
            return block_tmp;
        }
        block_tmp = get_next_element(block_tmp);
    }
    // // fprintf(stderr, "Empty list malloc %ld\n", size);
    return NULL;
}

static void *find_free_block(size_t size)
{
    if (*heap_start == 0) {
        // // fprintf(stderr, "Heap start next null malloc %ld\n", size);
        return NULL;
    }
    return traverse_free_list(size);
}

static void *add_free_block(size_t payload_size)
{
    size_t *current_break = get_current_break();
    boundary_tag_t *new_block_header_address = get_header(current_break);
    void *new_block_payload = NULL;
    size_t block_size = 0;
    size_t optional_padding_size = 0;
    void *optional_padding = NULL;

    if (new_block_header_address != NULL) {
        block_size = get_aligned_block_size(payload_size);
        init_boundary_tag(new_block_header_address, block_size);
        new_block_payload = allocate_block(block_size - BOUNDARY_TAG_SIZE);
        optional_padding_size = block_size - (BOUNDARY_TAG_SIZE * 2 + payload_size);
        optional_padding = add_optional_padding(payload_size, new_block_payload);
        init_boundary_tag(optional_padding + optional_padding_size, block_size);
        init_epilogue();
        return new_block_payload;
    }
    return NULL;
}

void *malloc(size_t size)
{
    void *free_block = NULL;
    size_t block_size = 0;

    init_heap();
    pthread_mutex_lock(&heap_start_mutex);
    if (heap_start == NULL) {
        pthread_mutex_unlock(&heap_start_mutex);
        return NULL;
    }
    block_size = get_aligned_block_size(size);
    free_block = find_free_block(block_size);
    if (free_block == NULL) {
        free_block = add_free_block(size);
        pthread_mutex_unlock(&heap_start_mutex);
        return free_block;
        // fprintf(stderr, "Add new block malloc %ld\n", size);
    }
    pthread_mutex_unlock(&heap_start_mutex);
    // if (check_heap() < 0 || check_free_list() < 0) {
    //     // fprintf(stderr, "ERROR\n");
    //     exit(1);
    // }
    return free_block;
}

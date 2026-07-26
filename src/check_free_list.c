#include "my_malloc.h"
// #include <stdio.h>

extern size_t *heap_start;

bool is_address_valid(size_t *block)
{
    return ((size_t)heap_start - BOUNDARY_TAG_SIZE <= (size_t)block) && ((size_t)block <= (size_t)get_current_break() && (size_t)block % ALIGNMENT_REQUIREMENT == 0);
}

static bool is_free_block_valid(size_t *payload_start)
{
    boundary_tag_t *header = get_header(payload_start);

    if (!is_block_valid(header)) {
        return false;
    }
    if (is_allocated(*header)) {
        return false;
    }
    return true;
}

static bool detect_cycle(size_t *start)
{
    size_t *slow = start;
    size_t *fast = start;
    size_t *next_fast = NULL;

    while (slow != NULL && fast != NULL) {
        slow = get_next_element(slow);
        next_fast = get_next_element(fast);
        if (slow == NULL || next_fast == NULL) {
            return false;
        }
        fast = get_next_element(next_fast);
        if (fast == NULL) {
            return false;
        }
        // fprintf(stderr, "Slow pointer %ld fast %ld || slow %ld fast %ld\n", (size_t)slow, (size_t)fast, *slow, *fast);
        if (*slow == *fast) {
            return true;
        }
    }
    return false;
}

int check_free_list()
{
    size_t *block_tmp = NULL;
    size_t total_free_blocks = 0;

    if (heap_start == NULL || *heap_start == 0) {
        return 0;
    }
    block_tmp = (size_t *)(*heap_start);
    if (detect_cycle(block_tmp)) {
        return -1;
    }
    while (block_tmp != NULL) {
        if (!is_free_block_valid(block_tmp)) {
            return -1;
        }
        total_free_blocks += 1;
        // fprintf(stderr, "Free block Header %ld || Size : %ld allocated %d // counter %ld \n", (size_t)get_header(block_tmp), get_size(*get_header(block_tmp)), is_allocated(*get_header(block_tmp)), total_free_blocks);
        block_tmp = get_next_element(block_tmp);
    }
    // fprintf(stderr, "Total free blocks heap %ld\n\n", total_free_blocks);
    return total_free_blocks;
}

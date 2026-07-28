#include "my_malloc.h"

extern char *heap_start;

bool is_address_valid(size_t block)
{
    return ((size_t)heap_start - BOUNDARY_TAG_SIZE <= block)
        && (block <= (size_t)get_current_break()
        && block % ALIGNMENT_REQUIREMENT == 0);
}

static bool is_free_block_valid(char *payload_start)
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

static bool detect_cycle(char *start)
{
    char *slow = start;
    char *fast = start;
    char *next_fast = NULL;

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
        if (*slow == *fast) {
            return true;
        }
    }
    return false;
}

__attribute__((visibility("default"))) int check_free_list()
{
    char *block_tmp = NULL;
    size_t total_free_blocks = 0;

    if (heap_start == NULL || *(size_t *)heap_start == 0) {
        return 0;
    }
    block_tmp = (char *)*(size_t *)heap_start;
    if (detect_cycle(block_tmp)) {
        return -1;
    }
    while (block_tmp != NULL) {
        if (!is_free_block_valid(block_tmp)) {
            return -1;
        }
        total_free_blocks += 1;
        block_tmp = get_next_element(block_tmp);
    }
    return total_free_blocks;
}

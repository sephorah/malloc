#include "my_malloc.h"
#include <stdlib.h>

extern size_t *heap_start;

bool is_address_valid(size_t *block)
{
    return ((size_t)heap_start - BOUNDARY_TAG_SIZE <= (size_t)block) && ((size_t)block <= (size_t)get_current_break());
}

static int check_errors_valid_list(size_t *block)
{
    boundary_tag_t *block_header = get_header(block); // header helper done function

    if (!is_address_valid(block))
    {
        return ERROR_CODE;
    }
    if (block_header && is_allocated(block_header))
    {
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static size_t *get_next_element(size_t *block)
{
    size_t *block_next = (size_t *)((size_t)block);

    if (block_next == NULL)
    {
        return NULL;
    }
    return (size_t *)(*block_next);
}

static bool detect_cycle(size_t *start)
{
    size_t *slow = start;
    size_t *fast = start;
    size_t *next_fast = NULL;

    while (slow != NULL && fast != NULL)
    {
        slow = get_next_element(slow);
        next_fast = get_next_element(fast);
        if (slow == NULL || next_fast == NULL)
        {
            return false;
        }
        fast = get_next_element(next_fast);
        if (fast == NULL)
        {
            return false;
        }
        if (*slow == *fast)
        {
            return true;
        }
    }
    return false;
}

int check_free_list()
{
    size_t *block_tmp = NULL;

    if (heap_start == NULL || *heap_start == 0)
    {
        return SUCCESS_CODE;
    }
    block_tmp = (size_t *)(*heap_start);
    if (detect_cycle(block_tmp))
    {
        return ERROR_CODE;
    }
    while (block_tmp != NULL)
    {
        if (check_errors_valid_list(block_tmp))
        {
            return ERROR_CODE;
        }
        block_tmp = get_next_element(block_tmp);
    }
    return SUCCESS_CODE;
}

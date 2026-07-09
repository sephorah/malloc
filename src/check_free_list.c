#include "my_malloc.h"
#include <stdlib.h>

extern size_t *heap_start;

bool is_address_valid(size_t *block_tmp)
{
    return (size_t)heap_start - BOUNDARY_TAG_SIZE <= (size_t)block_tmp && (size_t)block_tmp <= (size_t)get_current_break();
}

static int check_errors_valid_list(size_t *block_tmp)
{
    boundary_tag_t *block_tmp_header = (boundary_tag_t *)((size_t)block_tmp - BOUNDARY_TAG_SIZE);

    if (!is_address_valid(block_tmp))
    {
        fprintf(stderr, "Invalid address\n");
        return ERROR_CODE;
    }
    if (block_tmp_header && is_allocated(block_tmp_header))
    {
        fprintf(stderr, "Allocated block in free list\n");
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

int check_valid_list()
{
    size_t *block_tmp = NULL;
    size_t *block_tmp_next = NULL;

    if (heap_start == NULL || *heap_start == 0)
    {
        return SUCCESS_CODE;
    }
    block_tmp = (size_t *)(*heap_start);
    while (block_tmp != NULL)
    {
        if (check_errors_valid_list(block_tmp)) {
            return ERROR_CODE;
        }
        block_tmp_next = (size_t *)((size_t)block_tmp);
        block_tmp = (size_t *)(*block_tmp_next);
    }
    return SUCCESS_CODE;
}

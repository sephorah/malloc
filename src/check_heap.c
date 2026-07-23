#include "my_malloc.h"
#include <stdlib.h>

extern size_t *heap_start;

static int check_boundary_tag(boundary_tag_t *tag)
{
    if (!is_address_valid(tag))
    {
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static int check_tags_equality(boundary_tag_t *block_header, boundary_tag_t *block_footer)
{
    if (*block_header != *block_footer)
    {
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static int check_size(size_t size)
{
    if (size < EPILOGUE_SIZE || size % ALIGNMENT_REQUIREMENT != 0)
    {
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static int check_blocks(size_t *block_header)
{
    size_t size = 0;
    boundary_tag_t *block_footer = NULL;

    if (check_boundary_tag(block_header))
    {
        return ERROR_CODE;
    }
    size = get_size(block_header);
    if (check_size(size))
    {
        return ERROR_CODE;
    }
    block_footer = get_footer(block_header, size); // footer helper done function done
    if (check_boundary_tag(block_footer))
    {
        return ERROR_CODE;
    }
    if (check_tags_equality(block_header, block_footer))
    {
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

int check_heap(void)
{
    size_t *block_tmp_header = NULL;
    size_t size_tmp = 0;
    size_t current_break = (size_t)get_current_break();

    if (heap_start == NULL)
    {
        return SUCCESS_CODE;
    }
    block_tmp_header = get_header(heap_start); // header helper done function
    while ((size_t)block_tmp_header < current_break)
    {
        size_tmp = get_size(block_tmp_header);
        if (check_blocks(block_tmp_header))
        {
            return ERROR_CODE;
        }
        block_tmp_header = (size_t *)((size_t)block_tmp_header + size_tmp);
    }
    return SUCCESS_CODE;
}

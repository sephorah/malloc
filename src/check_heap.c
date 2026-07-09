#include "my_malloc.h"
#include <stdlib.h>

extern size_t *heap_start;

static int check_boundary_tag(boundary_tag_t *tag, bool is_header)
{
    char *tag_name = (is_header) ? "header" : "footer";

    if (!is_address_valid(tag))
    {
        fprintf(stderr, "Invalid %s\n", tag_name);
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static int check_tags_equality(boundary_tag_t *block_tmp_header, boundary_tag_t *block_tmp_footer)
{
    if (*block_tmp_header != *block_tmp_footer)
    {
        fprintf(stderr, "Header and footer are not identical\n");
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static int check_size(size_t size)
{
    if (size == 0 || size % 16 != 0)
    {
        fprintf(stderr, "Invalid size\n");
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

static int check_blocks(size_t *block_tmp_header, boundary_tag_t *block_tmp_footer)
{
    size_t size_tmp = 0;

    if (check_boundary_tag(block_tmp_header, true))
    {
        return ERROR_CODE;
    }
    size_tmp = get_size(block_tmp_header);
    if (check_size(size_tmp))
    {
        return ERROR_CODE;
    }
    block_tmp_footer = (boundary_tag_t *)((size_t)block_tmp_header + size_tmp - BOUNDARY_TAG_SIZE);
    if (check_boundary_tag(block_tmp_footer, false))
    {
        return ERROR_CODE;
    }
    if (check_tags_equality(block_tmp_header, block_tmp_footer))
    {
        return ERROR_CODE;
    }
    return SUCCESS_CODE;
}

int check_heap(void)
{
    size_t *block_tmp_header = NULL;
    size_t size_tmp = 0;
    boundary_tag_t *block_tmp_footer = NULL;
    size_t current_break = (size_t)get_current_break();

    if (heap_start == NULL)
    {
        return SUCCESS_CODE;
    }
    block_tmp_header = (boundary_tag_t *)((size_t)heap_start - BOUNDARY_TAG_SIZE);
    while ((size_t)block_tmp_header < current_break)
    {
        if (check_blocks(block_tmp_header, block_tmp_footer))
        {
            return ERROR_CODE;
        }
        block_tmp_header = (size_t *)((size_t)block_tmp_header + size_tmp);
    }
    return SUCCESS_CODE;
}

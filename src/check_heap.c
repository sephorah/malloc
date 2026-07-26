#include "my_malloc.h"
// #include <stdio.h>

extern size_t *heap_start;

static bool is_boundary_tag_valid(boundary_tag_t *tag)
{
    if (!is_address_valid(tag)) {
        return false;
    }
    return true;
}

static bool check_tags_equality(boundary_tag_t *block_header, boundary_tag_t *block_footer)
{
    if (*block_header != *block_footer) {
        return false;
    }
    return true;
}

static bool is_size_valid(size_t size)
{
    if (size < EPILOGUE_SIZE || size % ALIGNMENT_REQUIREMENT != 0) {
        return false;
    }
    return true;
}

bool is_block_valid(size_t *block_header)
{
    size_t size = 0;
    boundary_tag_t *block_footer = NULL;

    if (!is_boundary_tag_valid(block_header)) {
        // fprintf(stderr, "1 free\n");
        // fprintf(stderr, "Block header %ld\n", (size_t)block_header);
        return false;
    }
    size = get_size(*block_header);
    if (!is_size_valid(size)) {
        // fprintf(stderr, "2 free %ld\n", size);
        // fprintf(stderr, "Block header %ld\n", (size_t)block_header);
        return false;
    }
    block_footer = get_footer(block_header, size);
    if (!is_boundary_tag_valid(block_footer)) {
        // fprintf(stderr, "3 free\n");
        // fprintf(stderr, "Block header %ld %ld footer %ld\n", (size_t)block_header, *block_header, (size_t)block_footer);
        return false;
    }
    if (!check_tags_equality(block_header, block_footer)) {
        // fprintf(stderr, "4 free\n");
        // fprintf(stderr, "Block header %ld %ld footer %ld %ld\n", (size_t)block_header, *block_header, (size_t)block_footer, *block_footer);
        return false;
    }
    return true;
}

int count_free_blocks_heap(void)
{
    size_t *block_tmp_header = NULL;
    size_t size_tmp = 0;
    size_t current_break = (size_t)get_current_break();
    size_t total_free_blocks = 0;

    if (heap_start == NULL) {
        return 0;
    }
    block_tmp_header = get_header(heap_start);
    while ((size_t)block_tmp_header < current_break) {
        size_tmp = get_size(*block_tmp_header);
        if (!is_block_valid(block_tmp_header)) {
            return -1;
        }
        if (!is_allocated(*block_tmp_header)) {
            total_free_blocks += 1;
        }
        block_tmp_header = (size_t *)((size_t)block_tmp_header + size_tmp);
    }
    return total_free_blocks;
}

int check_heap(void)
{
    size_t *block_tmp_header = NULL;
    size_t size_tmp = 0;
    size_t current_break = (size_t)get_current_break();
    size_t total_blocks = 0;

    if (heap_start == NULL) {
        return 0;
    }
    block_tmp_header = get_header(heap_start);
    while ((size_t)block_tmp_header < current_break) {
        size_tmp = get_size(*block_tmp_header);
        if (!is_block_valid(block_tmp_header)) {
            return -1;
        }
        // fprintf(stderr, "Header %ld || Size : %ld allocated %d // counter %ld \n", (size_t)block_tmp_header, (size_t)size_tmp, is_allocated(*block_tmp_header), total_blocks);
        total_blocks += 1;
        block_tmp_header = (size_t *)((size_t)block_tmp_header + size_tmp);
    }
    // fprintf(stderr, "Total blocks heap %ld\n\n", total_blocks);
    return total_blocks;
}

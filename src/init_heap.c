#include "my_malloc.h"

extern char *heap_start;

void init_epilogue(void)
{
    boundary_tag_t *block_start = allocate_block(BOUNDARY_TAG_SIZE);

    if (block_start == NULL) {
        return;
    }
    init_boundary_tag(block_start, BOUNDARY_TAG_SIZE);
}

void init_heap(void)
{
    if (heap_start != NULL) {
        return;
    }
    heap_start = init_block(8);
    init_epilogue();
}
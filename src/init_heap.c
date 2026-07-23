#include "my_malloc.h"

extern size_t *heap_start;

void *init_epilogue(void)
{
    size_t *block_start = allocate_block(BOUNDARY_TAG_SIZE);

    if (block_start == NULL)
    {
        return NULL;
    }
    init_boundary_tag(block_start, BOUNDARY_TAG_SIZE); //change
    // *block_start = BOUNDARY_TAG_SIZE;
    // mark_boundary_tag_allocated(block_start);
    // *block_start = SET_BIT(*block_start, 0);
    return block_start;
}

void init_heap(void)
{
    if (heap_start != NULL) // change
    {
        return;
    }
    heap_start = init_block(8);
    init_epilogue();
}
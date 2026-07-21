#include "my_malloc.h"

extern size_t *heap_start;

void *init_epilogue(void)
{
    size_t *block_start = allocate_block(BOUNDARY_TAG_SIZE);

    if (block_start == NULL)
    {
        return NULL;
    }
    *block_start = BOUNDARY_TAG_SIZE;
    *block_start = SET_BIT(block_start, 0);
    return block_start;
}

void init_heap(void)
{
    if ((size_t *)heap_start != NULL)
    {
        return;
    }
    heap_start = init_block(8);
    init_epilogue();
}
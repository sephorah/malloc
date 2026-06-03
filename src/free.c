#include "my_malloc.h"

extern size_t *heap_start;

void update_free_list(size_t *ptr)
{
    size_t *ptr_prev = NULL;
    size_t *old_block_prev = NULL;
    size_t *old_block_next = NULL;

    if (heap_start == NULL)
    {
        return;
    }
    ptr_prev = (size_t *)(ptr + 1);
    fprintf(stderr, "BEFORE UPDATE FREE LIST\n");
    fprintf(stderr, "HEAD address %p payload %ld 0x%lx\n", heap_start, *heap_start, *heap_start);
    fprintf(stderr, "Ptr address %p next 0x%lx %ld prev %lx %ld\n", ptr, *ptr, *ptr, *ptr_prev, *ptr_prev);
    fprintf(stderr, "Old block address %ld 0x%lx\n", *heap_start, *heap_start);

    fprintf(stderr, "Hey prev %p\n", ptr_prev);
    if (*heap_start != 0)
    {
        *ptr = *heap_start;
        *ptr_prev = (size_t)heap_start;
        old_block_prev = (size_t *)(*heap_start + 1);
        old_block_next = (size_t *)(*heap_start);
        fprintf(stderr, "BEFORE OLD BLOCK PREVIOUS %p %ld 0x%lx NEXT %p %ld 0x%lx\n", old_block_prev, *old_block_prev, *old_block_prev, old_block_next, *old_block_next, *old_block_next);

        *old_block_prev = (size_t)ptr;
        fprintf(stderr, "AFTER OLD BLOCK PREVIOUS %p %ld 0x%lx NEXT %p %ld 0x%lx\n", old_block_prev, *old_block_prev, *old_block_prev, old_block_next, *old_block_next, *old_block_next);
    }
    else
    {
        fprintf(stderr, "BEFORE %lb %lb\n", *ptr, *(ptr + 8));
        *ptr = 0;
        *ptr_prev = 0;
        fprintf(stderr, "AFTER %lb %lb\n", *ptr, *(ptr + 8));
    }
    *heap_start = (size_t)ptr;
    fprintf(stderr, "AFTER UPDATE FREE LIST\n");
    fprintf(stderr, "HEAD address %p payload %ld 0x%lx\n", heap_start, *heap_start, *heap_start);
    fprintf(stderr, "Ptr address %p next 0x%lx %ld prev 0x%lx %ld\n", ptr, *ptr, *ptr, *ptr_prev, *ptr_prev);
    fprintf(stderr, "Old block address %ld 0x%lx\n", *heap_start, *heap_start);
}

void free(void *ptr)
{
    header_t *header_address = NULL;

    if (ptr == NULL)
    {
        // fprintf(stderr, "Null pointer\n");
        return;
    }
    header_address = ptr - HEADER_SIZE;
    if (!is_allocated(header_address))
    {
        // fprintf(stderr, "Already freed %p\n", header_address);
        return;
    }
    // fprintf(stderr, "Original address %p %ld\n", ptr, (size_t)ptr);
    // fprintf(stderr, "Before free %p %lb\n", header_address, *header_address);
    *header_address = *header_address & ~(1 << 0); // clear bit position 0 ; MAKE FUNCTION
    // fprintf(stderr, "After free %p %lb\n", header_address, *header_address);
    fprintf(stderr, "UPDATE FREE LIST %p\n", ptr);
    update_free_list(ptr);
}

/*
remove useless sbrk in malloc

*/
#include "my_malloc.h"

void free(void *ptr)
{
    header_t *header_address = NULL;

    if (ptr == NULL)
    {
        return;
    }
    header_address = ptr - HEADER_SIZE;
    // fprintf(stderr, "Original address %p %ld\n", ptr, (size_t)ptr);
    // fprintf(stderr, "Before free %p %lb\n", header_address, *header_address);
    *header_address = *header_address & ~(1 << 0); // clear bit position 0 ; MAKE FUNCTION
    // fprintf(stderr, "After free %p %lb\n", header_address, *header_address);
}
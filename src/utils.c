#include "my_malloc.h"

int is_allocated(header_t *header)
{
    return CHECK_BIT(*header, 0);
}

size_t get_size(header_t *header)
{
    return *header & ~(1 << 0); // clear bit position 0; MAKE FUNCTION
}

void *get_current_break(void)
{
    void *current_break = sbrk(0);

    if (current_break == (void *)-1)
    {
        return NULL;
    }
    return current_break;
}
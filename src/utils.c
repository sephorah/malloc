#include "my_malloc.h"

int is_allocated(boundary_tag_t header)
{
    return CHECK_BIT(header, 0);
}

size_t get_size(boundary_tag_t header)
{
    return CLEAR_BIT(header);
}

void *get_current_break(void)
{
    void *current_break = sbrk(0);

    if (current_break == (void *)-1) {
        return NULL;
    }
    return current_break;
}

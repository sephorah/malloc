#include "my_malloc.h"

size_t malloc_usable_size(void *ptr)
{
    boundary_tag_t *header = NULL;

    if (ptr == NULL) {
        return 0;
    }
    header = get_header(ptr);
    return get_size(*header);
}

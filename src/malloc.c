#include <unistd.h>
#include <stdio.h>

void *malloc(size_t size)
{
    void *prev_break = sbrk(size);

    if (prev_break == (void *)-1)
    {
        perror("Error sbrk");
        return NULL;
    }
    return prev_break;
}

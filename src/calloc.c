#include "my_malloc.h"

void *calloc(size_t nmemb, size_t size)
{
    void *memory = NULL;
    size_t total = 0;

    if (nmemb == 0 || size == 0) {
        return NULL;
    }
    if (__builtin_mul_overflow(nmemb, size, &total)) {
        return NULL;
    }
    total = nmemb * size;
    memory = malloc(total);
    if (memory == NULL) {
        return NULL;
    }
    memset(memory, 0, nmemb * size);
    // memset(memory, 0, nmemb);
    return memory;
}
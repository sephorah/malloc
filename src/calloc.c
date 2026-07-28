#include "my_malloc.h"

__attribute__((visibility("default"))) void *calloc(size_t nmemb, size_t size)
{
    void *memory = NULL;
    size_t total = 0;

    if (nmemb == 0 || size == 0) {
        return malloc(0);
    }
    if (__builtin_mul_overflow(nmemb, size, &total)) {
        return NULL;
    }
    memory = malloc(total);
    if (memory == NULL) {
        return NULL;
    }
    memset(memory, 0, total);
    return memory;
}
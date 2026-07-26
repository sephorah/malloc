#include "my_malloc.h"

size_t *heap_start = NULL;

pthread_mutex_t heap_start_mutex = PTHREAD_MUTEX_INITIALIZER;
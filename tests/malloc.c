#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <dlfcn.h>
#include <unistd.h>

void handle_error(char *str)
{
    perror(str);
    exit(1);
}

Test(malloc, simple_alloc_free)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    str = (*malloc)(52);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    strcpy(str, "Epitech");
    cr_assert_str_eq(str, "Epitech");

    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(malloc, size_max)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    str = (*malloc)(__SIZE_MAX__);
    cr_assert_eq(str, NULL);
    cr_assert_eq((*check_heap)(), 2);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(malloc, reuse_freed_block)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;
    char *temp = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    str = (*malloc)(8);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    temp = str;
    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    str = (*malloc)(8);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    cr_assert_eq(str, temp);

    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(malloc, size_zero_alloc)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    block = (*malloc)(0);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(malloc, large_alloc)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    block = (*malloc)(1000000);
    strcpy(block, "Epitech");
    cr_assert_str_eq(block, "Epitech");
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(malloc, aligned_address)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    block = (*malloc)(100);
    cr_assert(((size_t)block % 16) == 0);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(malloc, many_mallocs)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *current_break = NULL;
    char *new_current_break = NULL;
    char *blocks[30];

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    for (int i = 0; i < 20; i++) {
        blocks[i] = (*malloc)(100);
    }
    cr_assert_eq((*check_heap)(), 22);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    for (int i = 0; i < 8; i++) {
        (*free)(blocks[i]);
    }
    current_break = sbrk(0);
    cr_assert_eq((*check_heap)(), 15);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    for (int i = 20; i < 30; i++) {
        blocks[i] = (*malloc)(50);
    }
    new_current_break = sbrk(0);
    cr_assert_eq(current_break, new_current_break);
    cr_assert_eq((*check_heap)(), 25);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    for (int i = 8; i < 30; i++) {
        (*free)(blocks[i]);
    }
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    dlclose(handle);
}

Test(free, handle_null)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);

    if (!handle) {
        handle_error("Error dlopen");
    }
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(NULL);
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    dlclose(handle);
}

Test(free, double_free)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    count_free_blocks_heap = dlsym(handle, "count_free_blocks_heap");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }

    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    block = (*malloc)(70);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    dlclose(handle);
}

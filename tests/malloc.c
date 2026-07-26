#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <dlfcn.h>

int handle_error(char *str)
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
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    str = (*malloc)(52);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    strcpy(str, "Epitech");
    cr_assert_str_eq(str, "Epitech");
    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1, "got %d\n", (*check_free_list)());
    dlclose(handle);
}

Test(malloc, reuse_freed_block)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *str = NULL;
    char *temp = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    str = (*malloc)(8);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    temp = str;
    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1, "got %d\n", (*check_free_list)());
    str = (*malloc)(8);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq(str, temp);
    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    dlclose(handle);
}

Test(malloc, zero_size_alloc)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    block = (*malloc)(0);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1, "got %d\n", (*check_free_list)());
    dlclose(handle);
}

Test(malloc, large_alloc)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    block = (*malloc)(1000000);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1, "got %d\n", (*check_free_list)());
    dlclose(handle);
}

Test(malloc, aligned_address)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    block = (*malloc)(100);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1, "got %d\n", (*check_free_list)());
    dlclose(handle);
}

Test(malloc, many_mallocs)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *blocks[30];
    // // fprintf(stderr, "Many mallocs\n");
    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    for (int i = 0; i < 20; i++) {
        blocks[i] = (*malloc)(100);
        // // fprintf(stderr, "malloc block %ld %ld\n", i, blocks[i]);
    }
    cr_assert_eq((*check_heap)(), 22);
    cr_assert_eq((*check_free_list)(), 0);
    for (int i = 0; i < 8; i++) {
        (*free)(blocks[i]);
        // // fprintf(stderr, "free block %ld %ld\n", i, blocks[i]);
    }
    cr_assert_eq((*check_heap)(), 15);
    cr_assert_eq((*check_free_list)(), 1);
    for (int i = 20; i < 30; i++) {
        blocks[i] = (*malloc)(50);
    }
    cr_assert_eq((*check_heap)(), 25);
    cr_assert_eq((*check_free_list)(), 1);
    for (int i = 8; i < 30; i++) {
        (*free)(blocks[i]);
    }
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    // COMPARE TOTAL FREE BLOCKS IN FREE LIST AND HEAP
    dlclose(handle);
    // // fprintf(stderr, "End test\n");
}

Test(free, handle_null)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);

    if (!handle) {
        handle_error("Error dlopen");
    }
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    (*free)(NULL);
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    dlclose(handle);
}

Test(free, double_free)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *block = NULL;

    if (!handle) {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    block = (*malloc)(70);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    (*free)(block);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    dlclose(handle);
}

// test splitting reuse smaller blocks
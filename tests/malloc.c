#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <criterion/criterion.h>
#include <criterion/redirect.h>

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
    char *str = NULL;

    if (!handle)
    {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    str = (*malloc)(8);
    strcpy(str, "Epitech");
    cr_assert_str_eq(str, "Epitech");
    (*free)(str);
    dlclose(handle);
}

Test(malloc, reuse_freed_block)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    char *str = NULL;
    char *temp = NULL;

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    str = (*malloc)(8);
    temp = str;
    (*free)(str);
    str = (*malloc)(8);
    cr_assert_eq(str, temp);
    (*free)(str);
    dlclose(handle);
}

Test(malloc, zero_size_alloc)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    char *block = NULL;

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    block = (*malloc)(0);
    (*free)(block);
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

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    block = (*malloc)(1000000);
    (*free)(block);
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    dlclose(handle);
}

Test(malloc, aligned_address)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    char *block = NULL;

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    block = (*malloc)(100);
    cr_assert_eq((size_t)block % 16, 0);
    (*free)(block);
    dlclose(handle);
}

Test(malloc, many_mallocs)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    char *blocks[20];

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    check_heap = dlsym(handle, "check_heap");
    check_free_list = dlsym(handle, "check_free_list");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    for (int i = 0; i < 20; i++)
    {
        blocks[i] = (*malloc)(100);
    }
    for (int i = 0; i < 20; i++)
    {
        (*free)(blocks[i]);
    }
    for (int i = 0; i < 20; i++)
    {
        blocks[i] = (*malloc)(100);
    }
    cr_assert_eq((*check_heap)(), 0);
    cr_assert_eq((*check_free_list)(), 0);
    for (int i = 0; i < 20; i++)
    {
        (*free)(blocks[i]);
    }
    dlclose(handle);
}

Test(free, handle_null)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*free)(void *ptr);

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    free = dlsym(handle, "free");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    (*free)(NULL);
    dlclose(handle);
}

Test(free, double_free)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    char *block = NULL;

    if (!handle)
    {
        handle_error("Error dlopen");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    if (dlerror() != NULL)
    {
        handle_error("Error dlerror");
    }
    block = (*malloc)(70);
    (*free)(block);
    (*free)(block);
    dlclose(handle);
}

#include <string.h>
#include <stddef.h>
#include <dlfcn.h>
#include <criterion/criterion.h>
#include <criterion/redirect.h>

Test(malloc_tests, strcpy_test)
{
    void *handle;
    void *(*malloc)(size_t size);
    char *str = NULL;

    handle = dlopen("./libmalloc.so", RTLD_LAZY);
    if (!handle)
    {
        return;
    }
    malloc = dlsym(handle, "malloc");
    if (dlerror() != NULL)
    {
        return;
    }
    str = malloc(sizeof(char) * 14);
    strcpy(str, "Hello, World!");
    cr_assert_str_eq(str, "Hello, World!");
    dlclose(handle);
}


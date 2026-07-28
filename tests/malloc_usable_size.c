#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <dlfcn.h>
#include <unistd.h>

void handle_error(char *str);

Test(malloc_usable_size, simple_alloc_free)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*free)(void *ptr);
    size_t *(*malloc_usable_size)(void *ptr);
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    free = dlsym(handle, "free");
    malloc_usable_size = dlsym(handle, "malloc_usable_size");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    str = (*malloc)(52);
    cr_assert_eq((*malloc_usable_size)(str), 64);
    (*free)(str);
    dlclose(handle);
}

Test(malloc_usable_size, null_ptr)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    size_t *(*malloc_usable_size)(void *ptr);
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc_usable_size = dlsym(handle, "malloc_usable_size");
    if (dlerror() != NULL) {
        handle_error("Error dlerror");
    }
    cr_assert_eq((*malloc_usable_size)(NULL), 0);
    dlclose(handle);
}

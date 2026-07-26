#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <dlfcn.h>
#include <unistd.h>

int handle_error(char *str);

Test(realloc, shrink)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    void *(*malloc)(size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;
    char *current_break = NULL;
    char *new_current_break = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    realloc = dlsym(handle, "realloc");
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
    current_break = sbrk(0);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    strcpy(str, "Epitech");
    cr_assert_str_eq(str, "Epitech");

    (*realloc)(str, 4);
    new_current_break = sbrk(0);
    cr_assert_str_eq(str, "Epitech");
    cr_assert_eq(current_break, new_current_break);
    cr_assert_eq((*check_heap)(), 4);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(realloc, realloc_null)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY | RTLD_DEEPBIND);
    void *(*free)(void *ptr);
    void *(*realloc)(void *ptr, size_t size);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    realloc = dlsym(handle, "realloc");
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

    str = (*realloc)(NULL, 52);
    cr_assert_eq((*check_heap)(), 3, "got %ld\n", (*check_heap)());
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

Test(realloc, size_zero)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY | RTLD_DEEPBIND);
    void *(*malloc)(size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    realloc = dlsym(handle, "realloc");
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

    str = (*malloc)(10);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    str = (*realloc)(str, 0);
    cr_assert_eq(str, NULL);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(realloc, grow)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY | RTLD_DEEPBIND);
    void *(*malloc)(size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    char *str = NULL;
    char *new_str = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    malloc = dlsym(handle, "malloc");
    realloc = dlsym(handle, "realloc");
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

    str = (*malloc)(100);
    strcpy(str, "Epitech");
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    new_str = (*realloc)(str, 170);
    cr_assert_str_eq(new_str, "Epitech");
    cr_assert_eq((*check_heap)(), 4);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(new_str);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}
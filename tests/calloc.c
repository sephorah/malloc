#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <dlfcn.h>
#include <unistd.h>

void handle_error(char *str);

Test(calloc, simple_calloc)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY | RTLD_DEEPBIND);
    void *(*calloc)(size_t nmemb, size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    int *array = NULL;
    int expected_null_array[15] = {0};
    int expected_array[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    if (!handle) {
        handle_error("Error handle");
    }
    calloc = dlsym(handle, "calloc");
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

    array = calloc(15, sizeof(int));
    cr_assert_arr_eq(array, expected_null_array, 15 * sizeof(int));
    for (int i = 0; i < 15; i++) {
        array[i] = i + 1;
    }
    cr_assert_arr_eq(array, expected_array, 15 * sizeof(int));
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(array);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(calloc, size_zero)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY | RTLD_DEEPBIND);
    void *(*calloc)(size_t nmemb, size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    int *array = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    calloc = dlsym(handle, "calloc");
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

    array = calloc(20, 0);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(array);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

Test(calloc, nmemb_zero)
{
    void *handle = dlopen("./libmalloc.so", RTLD_LAZY | RTLD_DEEPBIND);
    void *(*calloc)(size_t nmemb, size_t size);
    void *(*free)(void *ptr);
    int (*check_heap)(void);
    int (*check_free_list)(void);
    int (*count_free_blocks_heap)(void);
    int *array = NULL;

    if (!handle) {
        handle_error("Error handle");
    }
    calloc = dlsym(handle, "calloc");
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

    array = calloc(0, sizeof(char));
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 0);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());

    (*free)(array);
    cr_assert_eq((*check_heap)(), 3);
    cr_assert_eq((*check_free_list)(), 1);
    cr_assert_eq((*count_free_blocks_heap)(), (*check_free_list)());
    dlclose(handle);
}

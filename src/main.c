#include <stdio.h>

int main(void)
{
    // void *handle = dlopen("./libmalloc.so", RTLD_LAZY);
    // void *malloc(size_t size);
    // void *free(void *ptr);
    // int (*check_heap)(void);
    // int (*check_free_list)(void);
    char *blocks[30];
    // // fprintf(stderr, "Many mallocs\n");
    //  if (!handle) {
    //      // handle_error("Error dlopen");
    //  }
    //  malloc = dlsym(handle, "malloc");
    //  free = dlsym(handle, "free");
    //  check_heap = dlsym(handle, "check_heap");
    //  check_free_list = dlsym(handle, "check_free_list");
    //  if (dlerror() != NULL) {
    //      // handle_error("Error dlerror");
    //  }
    //  cr_assert_eq((*check_heap)(), 0);
    //  cr_assert_eq((*check_free_list)(), 0);
    for (int i = 0; i < 20; i++) {
        blocks[i] = malloc(100);
        // // fprintf(stderr, "malloc block %ld %ld\n", i, blocks[i]);
    }
    // cr_assert_eq((*check_heap)(), 22, "1heap expected 22 got %d\n", (*check_heap)());
    // cr_assert_eq((*check_free_list)(), 0, "1free list expected 0 got %d\n", (*check_free_list)());
    for (int i = 0; i < 8; i++) {
        free(blocks[i]);
        // // fprintf(stderr, "free block %ld %ld\n", i, blocks[i]);
    }
    // cr_assert_eq((*check_heap)(), 15, "2heap expected 15 got %d\n", (*check_heap)());
    // cr_assert_eq((*check_free_list)(), 1, "2free list expected 8 got %d\n", (*check_free_list)());
    for (int i = 20; i < 30; i++) {
        blocks[i] = malloc(50);
    }
    // cr_assert_eq((*check_heap)(), 24, "3heap expected 24 got %d\n", (*check_heap)());
    // cr_assert_eq((*check_free_list)(), 0, "3free list expected 0 got %d\n", (*check_free_list)());
    for (int i = 8; i < 30; i++) {
        free(blocks[i]);
    }
    // cr_assert_eq((*check_heap)(), 25, "4heap expected 24 got %d\n", (*check_heap)());
    // cr_assert_eq((*check_free_list)(), 0, "4free list expected 24 got %d\n", (*check_free_list)());
    //  COMPARE TOTAL FREE BLOCKS IN FREE LIST AND HEAP
    // dlclose(handle);
    return 0;
}
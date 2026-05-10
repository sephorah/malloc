// #include "my_malloc.h"

// void my_print(char *str)
// {
//     write(1, "break : ", strlen(str));
// }


// void show_alloc_mem(void)
// {
//     header_t *block_tmp = heap_start;
//     size_t size_tmp = get_size(block_tmp);
//     void *current_break = get_current_break();

//     if (current_break != NULL) {
//         my_print("break : ");
//         my_print()
//     }
//     while (size_tmp != 0)
//     {
//         size_tmp = get_size(block_tmp);
//         fprintf(stderr, "Before Current block header %p %ld %lb\n", block_tmp, *(header_t *)block_tmp, *(header_t *)block_tmp);
//         fprintf(stderr, "Current block size %p %ld %lb\n", block_tmp, size_tmp, size_tmp);
//         block_tmp = block_tmp + size_tmp;
//         fprintf(stderr, "After current block header %p %ld %lb\n", block_tmp, *(header_t *)block_tmp, *(header_t *)block_tmp);

//         break : 0xB0000
//         2 0xA0000 - 0xA41CA : 16842 octets
//         3 0xAE000 - 0xAE03F : 63 octets
//     }
// }
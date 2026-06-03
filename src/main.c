#include "my_malloc.h"
#include <stdlib.h>

int main(void)
{
    // void *heap_start = sbrk(0);

    // if (heap_start == (void *)-1)
    // {
    //     fprintf(stderr, " failed");
    //     return 1;
    // }

    // fprintf(stderr, "First malloc\n");
    // void *current = sbrk(0);
    // fprintf(stderr, "Current break %p %ld\n", current, current);
    // char *str = malloc(sizeof(char) * 0);
    // if (str == NULL)
    // {
    //     fprintf(stderr, "Malloc failed\n");
    //     return 1;
    // }
    // else
    // {
    //     strcpy(str, "Hello !");
    //     strcpy(str, "Hello, World!");
    //     fprintf(stderr, "STR [%s]\n", str);
    //     // fprintf(stderr, "Address %p %ld difference %p %ld\n", str, str, (size_t)str - (size_t)current, (size_t)str - (size_t)current);
    //     // free(str);
    //     // free(str);
    //     // perror("Error ");
    // }




    fprintf(stderr, "First malloc\n");
    char *str = malloc(sizeof(char) * 14);
    if (str == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return 1;
    }
    else
    {
        // strcpy(str, "Hello !");
        strcpy(str, "Hello, World!");
        fprintf(stderr, "STR [%s]\n", str);
        // free(str);
    }

    fprintf(stderr, "\nSecond malloc\n");
    char *str2 = malloc(sizeof(char) * 14);
    if (str2 == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return 1;
    }
    else
    {
        // strcpy(str2, "12345Wo");
        strcpy(str2, "12345, World!");
        fprintf(stderr, "STR 2 [%s]\n", str2);
        free(str2);
    }

    fprintf(stderr, "\nThird malloc\n");
    char *str3 = malloc(sizeof(char) * 14);
    if (str3 == NULL)
    {
        fprintf(stderr, "Malloc failed\n");
        return 1;
    }
    else
    {
        // strcpy(str2, "12345Wo");
        strcpy(str3, "78910, World!");
        fprintf(stderr, "STR 3 [%s]\n", str3);
        // free(str3);
    }

    fprintf(stderr, "FREE STR3\n");
    free(str3);
    fprintf(stderr, "FREE STR2\n");
    free(str2);



    // realloc growth loop — what ls does to entry buffers
    // char *p = malloc(8);
    // for (int i = 0; i < 100; i++) p = realloc(p, 8 * (i + 2));
    // // free-then-reuse loop — forces find_free_block
    // void *a[50];
    // for (int i = 0; i < 50; i++) a[i] = malloc(16 + (i % 7) * 8);
    // for (int i = 0; i < 50; i += 2) free(a[i]);
    // for (int i = 0; i < 50; i += 2) a[i] = malloc(16 + (i % 5) * 8);

    // if (str == NULL)
    // {
    //     printf("Malloc failed");
    //     return 0;
    // }
    // strcpy(str, "Hello, World!");
    // if (strcmp(str, "Hello, World!") != 0)
    // {
    //     printf("Strings aren't equal\n");
    //     return 1;
    // }
    // free(str);
    return 0;
}
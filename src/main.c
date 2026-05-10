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
        free(str);
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
        // free(str2);
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
        free(str3);
    }


    // if (str == NULL)
    // {
    //     printf("Malloc failed");
    //     return;
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
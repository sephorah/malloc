#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <criterion/criterion.h>
#include <criterion/redirect.h>

// Test(free_list, strlen_tests)
// {
//     // void *handle;
//     // void *malloc(size_t size);

//     // void *handle;
//     // size_t (*strlen)(const char *s);
//     // char *str = "Epitech";
//     // char *str2 = "Taoyota";
//     // char *str3 = "Epitodoo";
//     // char *str4 = "Epot";

//     // handle = dlopen("./libasm.so", RTLD_LAZY);
//     // if (!handle)
//     // {
//     //     return;
//     // }
//     // strlen = dlsym(handle, "strlen");
//     // if (dlerror() != NULL)
//     // {
//     //     return;
//     // }
//     // cr_assert_eq((*strlen)(str), 7);
//     // cr_assert_eq((*strlen)(str2), 7);
//     // cr_assert_eq((*strlen)(str3), 8);
//     // cr_assert_eq((*strlen)(str4), 4);
//     // cr_assert_eq((*strlen)("str"), 3);
//     // cr_assert_eq((*strlen)(""), 0);
//     // dlclose(handle);
// }
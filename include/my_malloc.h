#ifndef MY_MALLOC_H_
#define MY_MALLOC_H_
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#define HEADER_SIZE 8
#define ALIGNMENT_REQUIREMENT 8
#define CHECK_BIT(number, position) ((number) & (1 << position))

typedef size_t header_t;

// typedef struct block
// {
//     uint64_t size; // 64...3 [block size] 2 ... 0 [0 0 a] a = 1 : allocated ; a = 0 : free
//     // struct block_header *prev;
//     void *payload;
// } block_t;

#endif /*MY_MALLOC_H_*/

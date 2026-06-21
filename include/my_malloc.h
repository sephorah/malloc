#ifndef MY_MALLOC_H_
#define MY_MALLOC_H_
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define HEADER_SIZE 16
#define ALIGNMENT_REQUIREMENT 16
#define CHECK_BIT(number, position) ((number) & (1 << position))
#define CLEAR_BIT(number) (number & ~(1 << 0))
#define SET_BIT(number, position) (*(boundary_tag_t *)number | (1 << position))

typedef size_t boundary_tag_t;

int is_allocated(boundary_tag_t *header);

size_t get_size(boundary_tag_t *header);

void *get_current_break(void);

void *malloc(size_t size);

void free(void *ptr);

void check_valid_list();

#endif /*MY_MALLOC_H_*/

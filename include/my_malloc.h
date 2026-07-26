#ifndef MY_MALLOC_H_
#define MY_MALLOC_H_

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <pthread.h>

#define BOUNDARY_TAG_SIZE 16
#define ALIGNMENT_REQUIREMENT 16
#define EPILOGUE_SIZE 16
#define MIN_BLOCK_SIZE 48
#define CHECK_BIT(number, position) ((number) & (1 << position))
#define CLEAR_BIT(number) (number & ~(1 << 0))
#define SET_BIT(number, position) (number | (1 << position))

extern pthread_mutex_t heap_start_mutex;

typedef size_t boundary_tag_t;

int is_allocated(boundary_tag_t header);

size_t get_size(boundary_tag_t header);

void *get_current_break(void);

void *malloc(size_t size);

void free(void *ptr);

size_t *merge_free_blocks(size_t *current_block);

void *init_epilogue(void);

void init_heap(void);

void *init_block(size_t payload_size);

boundary_tag_t *init_boundary_tag(boundary_tag_t *tag_start, size_t size);

size_t get_aligned_block_size(size_t payload_size);

void *allocate_block(size_t size);

size_t *add_optional_padding(size_t payload_size, void *start);

bool is_address_valid(size_t *block_tmp);

int check_heap(void);

int check_free_list(void);

boundary_tag_t *get_header(size_t *block_tmp);

boundary_tag_t *get_footer(size_t *header, size_t size);

void mark_boundary_tag_allocated(boundary_tag_t *tag);

void mark_boundary_tag_free(boundary_tag_t *tag);

int count_free_blocks_heap(void);

void add_block_free_list(size_t *ptr);

bool is_block_valid(size_t *block_header);

void remove_block_free_list(size_t *block_payload);

void handle_leftover_space(size_t *header, size_t new_size);

size_t *get_next_element(size_t *block);

size_t malloc_usable_size(void *ptr);

#endif /*MY_MALLOC_H_*/

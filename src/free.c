#include "my_malloc.h"
#include <stdlib.h>
extern size_t *heap_start;

// void check_boundary_tags(header_t *header, header_t *footer)
// {
//     return *header == *footer;
// }

void check_valid_list()
{
    size_t *block_tmp = NULL;
    size_t *block_tmp_prev = NULL;
    size_t *block_tmp_next = NULL;

    if (*heap_start == 0)
    {
        return;
    }
    fprintf(stderr, "CHECK VALID LIST\nFirst element %ld\n", *heap_start);
    block_tmp = (size_t *)(*heap_start);
    while (block_tmp != NULL)
    {
        block_tmp_prev = (size_t *)(block_tmp + 1);
        block_tmp_next = (size_t *)((size_t)block_tmp);
        fprintf(stderr, "Current block %ld is allocated %d size %ld next %ld prev %ld\n", (size_t)block_tmp, is_allocated((header_t *)((size_t)block_tmp - HEADER_SIZE)), get_size((header_t *)((size_t)block_tmp - HEADER_SIZE)), (*block_tmp_next), (*block_tmp_prev));

        // CHECK IF HEADER == FOOTER;

        // if (size_tmp >= size)
        // {
        //     block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
        //     *(header_t *)block_tmp_header = *(header_t *)block_tmp_header | (1 << 0); // MAKE FUNCTION FOR SETTING AND CLEAR BIT
        //     size_tmp = get_size(block_tmp_header);
        //     // fprintf(stderr, "[reused block] %p (%ld) - %p (%ld) : %ld bytes\n", block_tmp_header, (size_t)block_tmp_header, (size_t *)((size_t)block_tmp_header + size_tmp), (size_t)((size_t)block_tmp_header + size_tmp), size_tmp);

        //     *heap_start = *block_tmp_next;
        //     return block_tmp;
        // }
        block_tmp = (size_t *)(*block_tmp_next);
    }
    fprintf(stderr, "---------- NO PROBLEM\n");
    // return NULL;
}

void update_next_neighbor(size_t *block_header)
{
    // remove dangling absorbed current block
    size_t *current_next_block = (size_t *)*((size_t *)((size_t)block_header + HEADER_SIZE));
    size_t *new_next_block = NULL;

    fprintf(stderr, "In update next neighbor %ld %ld\n", (size_t)current_next_block, (size_t)current_next_block);

    if ((size_t)current_next_block == 0)
    {
        return;
    }
    new_next_block = (size_t *)*current_next_block;

    fprintf(stderr, "Update next neighbor block header %ld current next block %ld size %ld new next block %ld\n", (size_t)block_header, (size_t)current_next_block, get_size(current_next_block), (size_t)new_next_block);
    // exit(0);
    *current_next_block = *new_next_block;
    fprintf(stderr, "After update current next block value %ld\n", *current_next_block);
}

void check_next_block(size_t *current_block_header, size_t *current_block_footer)
{
    fprintf(stderr, "\ncurrent block header %ld footer %ld\n", (size_t)current_block_header, (size_t)current_block_footer);
    header_t *next_block_header = (size_t *)*(size_t *)((size_t)current_block_header + HEADER_SIZE);
    header_t *next_block_footer = NULL;

    if ((size_t)next_block_header != ((size_t)current_block_footer + HEADER_SIZE))
    {
        return;
    }
    //  if ((size_t)prev_block + get_size((prev_block - HEADER_SIZE)) != (size_t)current_block - HEADER_SIZE) {
    //     return current_block;
    // }

    next_block_footer = (header_t *)((size_t)next_block_header + get_size(next_block_header) - HEADER_SIZE);

    fprintf(stderr, "Next header %ld footer %ld size %ld %ld\n", (size_t)next_block_header, (size_t)next_block_footer, get_size(next_block_header), get_size(next_block_footer));
    *(size_t *)(next_block_header + HEADER_SIZE) = 0;
    *(size_t *)(next_block_header + HEADER_SIZE + 8) = 0;
    fprintf(stderr, "HEYYYYYYYYY next %ld prev %ld\n", *(size_t *)(next_block_header + HEADER_SIZE), *(size_t *)(next_block_header + HEADER_SIZE + 8));
    // fprintf(stderr, "HEEEEEEEEEEEEEEEEEEY %ld %ld\n", get_size(next_block_header), get_size(current_block_header));

    fprintf(stderr, "Next block header %ld size next block %ld footer %ld size footer %ld\n", (size_t)next_block_header, get_size(next_block_header), (size_t)next_block_footer, get_size(next_block_footer));
    *next_block_footer = get_size(next_block_header) + get_size(current_block_header);
    *current_block_header = *next_block_footer;
    update_next_neighbor(current_block_header);
    check_valid_list();
    fprintf(stderr, "New size next block footer %ld %ld\n", *next_block_footer, *current_block_header);

    // fprintf(stderr, "New sizes %ld %ld\n", *next_block_footer, *current_block_header);

    *next_block_header = 0;
    *current_block_footer = 0;

    // exit(1);
}

size_t *merge_free_blocks(size_t *current_block)
{
    header_t *prev_block_footer = NULL;
    header_t *prev_block_header = NULL;
    header_t *current_block_header = NULL;
    header_t *current_block_footer = NULL;
    size_t *prev_block = (size_t *)*(current_block + 1);
    size_t *next_block = (size_t *)*current_block;

    // size_t *updated_current_block = current_block;

    // header_t *prev_block_footer = (header_t *)((size_t)current_block - HEADER_SIZE * 2);
    // header_t *prev_block_header = (header_t *)((size_t)prev_block_footer - get_size(prev_block_footer) + HEADER_SIZE);
    // header_t *current_block_header = (header_t *)((size_t)current_block - HEADER_SIZE);
    // header_t *current_block_footer = (header_t *)((size_t)current_block + get_size(current_block_header) - HEADER_SIZE * 2);

    // Voir si les addresses sont correctes + si le footer est bien pris en compte dans malloc
    if (prev_block != 0) {
        if ((size_t)prev_block + get_size((size_t *)(*prev_block - HEADER_SIZE)) != (size_t)current_block - HEADER_SIZE)
        {
            return current_block;
        }
        prev_block_footer = (header_t *)((size_t)current_block - HEADER_SIZE * 2);
        prev_block_header = (header_t *)((size_t)prev_block_footer - get_size(prev_block_footer) + HEADER_SIZE);
        current_block_header = (header_t *)((size_t)current_block - HEADER_SIZE);
        current_block_footer = (header_t *)((size_t)current_block + get_size(current_block_header) - HEADER_SIZE * 2);
        fprintf(stderr, "\n\nPREV FREE | current block %ld\n", (size_t)current_block);
    
        fprintf(stderr, "Prev block header %ld size prev block %ld footer %ld size footer %ld\n", (size_t)prev_block_header, get_size(prev_block_header), (size_t)prev_block_footer, get_size(prev_block_footer));
        fprintf(stderr, "Current block header %ld size prev block %ld footer %ld size footer %ld\n", (size_t)current_block_header, get_size(current_block_header), (size_t)current_block_footer, get_size(current_block_footer));
    
        *current_block_footer = get_size(prev_block_footer) + get_size(current_block_header);
        *prev_block_header = *current_block_footer;
    
        fprintf(stderr, "Old current size %ld\n", get_size(current_block_header));
        // *prev_block_footer = 0;
        // *current_block_header = 0;
    
        *current_block = 0;
        *(current_block + 1) = 0;
        fprintf(stderr, "BEFORE CURRENT BLOCK header %ld payload %ld end %ld\n", (size_t)current_block_header, (size_t)current_block, (size_t)current_block_footer);
        current_block = (size_t *)((size_t)prev_block_header + HEADER_SIZE);
    
        current_block_header = prev_block_header;
        update_next_neighbor(current_block_header);
        // *current_block = 0;
        // *(current_block + 1) = 0;
        check_valid_list();
        fprintf(stderr, "Prev block header %ld size prev block %ld footer %ld size footer %ld\n", (size_t)prev_block_header, get_size(prev_block_header), (size_t)prev_block_footer, get_size(prev_block_footer));
    
        fprintf(stderr, "AFTER CURRENT BLOCK header %ld payload %ld end %ld\n", (size_t)current_block_header, (size_t)current_block, (size_t)current_block_footer);
        fprintf(stderr, "AGAIN next %ld prev %ld", (*current_block), *(current_block + 1));
    
        fprintf(stderr, "Values Current block header %ld size prev block %ld footer %ld size footer %ld\n", *current_block_header, get_size(current_block_header), *prev_block_footer, get_size(prev_block_footer));
        fprintf(stderr, "New block size %ld %ld\n", *current_block_footer, *prev_block_header);
        // exit(0);
    }

    // current_block_footer =
    // if (current_block)
    if (next_block != 0) {
        check_next_block(current_block_header, current_block_footer);
    }
    // fprintf(stderr, "AFTER CURRENT BLOCK header %p payload %p end %p\n\n", current_block_header, updated_current_block, current_block_footer);
    return current_block;
}

void add_block_free_list(size_t *ptr)
{
    size_t *ptr_prev = NULL;
    size_t *old_block_prev = NULL;
    // size_t *old_block_next = NULL;
    fprintf(stderr, "Adding %ld block\n", (size_t)ptr);
    if (heap_start == NULL)
    {
        return;
    }
    ptr_prev = (size_t *)(ptr + 1);
    fprintf(stderr, "before %ld prev %ld\n", *ptr, *ptr_prev);
    if (*heap_start != 0)
    {
        fprintf(stderr, "HEAP START %ld VALUE %ld + 8 %ld\n", (size_t)heap_start, *heap_start, (*heap_start + 8));
        *ptr = *heap_start;
        *ptr_prev = (size_t)heap_start;
        old_block_prev = (size_t *)(*heap_start + 8);
        // old_block_next = (size_t *)(*heap_start);
        *old_block_prev = (size_t)ptr;
    }
    else
    {
        *ptr = 0;
        *ptr_prev = 0;
    }
    // fprintf(stderr, "After Next %ld prev %ld\n", *ptr, *ptr_prev);
    *heap_start = (size_t)ptr;
    // check_valid_list();
    // fprintf(stderr, "UPDATED HEAP START VALUE %ld\n\n", *heap_start);
}

void free(void *ptr)
{
    // fprintf(stderr, "------------FREE WITH %ld\n", (size_t)ptr);
    header_t *header_address = NULL;
    // size_t *updated_ptr = NULL;
    // check_valid_list();
    if (ptr == NULL)
    {
        return;
    }
    header_address = ptr - HEADER_SIZE;
    if (!is_allocated(header_address))
    {
        return;
    }
    // fprintf(stderr, "[freed block] %p (%ld) : %ld bytes\n", header_address, (size_t)header_address, get_size(header_address));
    *header_address = CLEAR_BIT(*header_address);
    add_block_free_list(ptr);
    merge_free_blocks(ptr);
}

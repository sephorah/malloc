#include "my_malloc.h"

extern size_t *heap_start;

size_t get_aligned_block_size(size_t payload_size)
{
    size_t total_size = HEADER_SIZE + payload_size;
    size_t div = total_size / ALIGNMENT_REQUIREMENT;
    size_t res = div * ALIGNMENT_REQUIREMENT;

    if (total_size % ALIGNMENT_REQUIREMENT != 0)
    {
        res += ALIGNMENT_REQUIREMENT;
    }
    return res;
}

void *add_optional_padding(size_t size)
{
    void *optional_padding = sbrk(size);

    if (optional_padding == (void *)-1)
    {
        perror("Error when allocating optional padding");
        return NULL;
    }
    // fprintf(stderr, "\t[optional padding] %p (%ld) - %p (%ld)\n", optional_padding, (size_t)optional_padding, (size_t *)((size_t)optional_padding + size), ((size_t)optional_padding + size));
    return optional_padding;
}

void *find_free_block(size_t size)
{
    size_t *block_tmp = NULL;
    header_t *block_tmp_header = NULL;
    // size_t *block_tmp_prev = NULL;
    size_t *block_tmp_next = NULL;
    size_t size_tmp = 0;

    if (*heap_start == 0)
    {
        return NULL;
    }
    block_tmp = (size_t *)(*heap_start);
    block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
    size_tmp = get_size(block_tmp_header);

    while (block_tmp != NULL)
    {
        // block_tmp_prev = (size_t *)(block_tmp + 1);
        block_tmp_next = (size_t *)((size_t)block_tmp);

        if (size_tmp >= size)
        {
            block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
            *(header_t *)block_tmp_header = *(header_t *)block_tmp_header | (1 << 0); // MAKE FUNCTION FOR SETTING AND CLEAR BIT
            size_tmp = get_size(block_tmp_header);
            fprintf(stderr, "[reused block] %p (%ld) - %p (%ld) : %ld bytes\n", block_tmp_header, (size_t)block_tmp_header, (size_t *)((size_t)block_tmp_header + size_tmp), (size_t)((size_t)block_tmp_header + size_tmp), size_tmp);

            *heap_start = *block_tmp_next;
            return block_tmp;
        }
        block_tmp = (size_t *)(*block_tmp_next);
        block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
    }
    return NULL;
}

header_t *init_header(size_t size)
{
    header_t *header_start = sbrk(HEADER_SIZE);

    if (header_start == (void *)-1)
    {
        perror("Error when allocating header");
        return NULL;
    }
    *header_start = size;
    // fprintf(stderr, "\t[header] %p (%ld) - %p (%ld): %ld %lb\n", header_start, (size_t)header_start, (size_t *)((size_t)header_start + HEADER_SIZE), ((size_t)header_start + HEADER_SIZE), *header_start, *header_start);
    return header_start;
}

void *init_payload(size_t size, void *header_start)
{
    void *payload_start = sbrk(size);

    if (payload_start == (void *)-1)
    {
        perror("Error when allocating payload");
        return NULL;
    }
    *(header_t *)header_start = *(header_t *)header_start | (1 << 0); // set bit position 0
    // fprintf(stderr, "\t[payload] %p (%ld) - %p (%ld)\n", payload_start, (size_t)payload_start, (size_t *)((size_t)payload_start + size), ((size_t)payload_start + size));
    return payload_start;
}

void *init_block(size_t payload_size)
{
    size_t block_size = get_aligned_block_size(payload_size);
    size_t optional_padding_size = block_size - (HEADER_SIZE + payload_size);
    header_t *header_start = init_header(block_size);
    size_t *block_end = (size_t *)((size_t)header_start + block_size);
    void *payload_start = NULL;
    void *optional_padding = NULL;

    if (header_start == NULL)
    {
        return NULL;
    }
    payload_start = init_payload(payload_size, header_start);
    if (payload_start == NULL)
    {
        return NULL;
    }
    optional_padding = add_optional_padding(optional_padding_size);
    if (optional_padding == NULL)
    {
        return NULL;
    }
    fprintf(stderr, "[new block] %p (%ld) - %p (%ld): %ld bytes\n", header_start, (size_t)header_start, block_end, (size_t)block_end, block_size);
    return payload_start;
}

void *add_free_block(size_t payload_size)
{
    header_t *new_block_header_address = (header_t *)((size_t)get_current_break() - HEADER_SIZE);
    void *new_heap_end = NULL;
    void *new_block_payload = NULL;
    size_t *block_end = NULL;
    size_t block_size = 0;
    size_t optional_padding_size = 0;

    if (new_block_header_address != NULL)
    {
        block_size = get_aligned_block_size(payload_size);
        block_end = (size_t *)((size_t)new_block_header_address + block_size);
        *new_block_header_address = block_size;
        new_block_payload = init_payload(payload_size, new_block_header_address);
        optional_padding_size = block_size - (HEADER_SIZE + payload_size);
        add_optional_padding(optional_padding_size);
        fprintf(stderr, "[heap end into new block] %p (%ld) - %p (%ld): %ld bytes\n", new_block_header_address, (size_t)new_block_header_address, block_end, (size_t)block_end, block_size);
        new_heap_end = init_block(0);
        fprintf(stderr, "heap end : %p\n", (size_t *)((size_t)new_heap_end - HEADER_SIZE));
        return new_block_payload;
    }
    return NULL;
}

void init_heap(void)
{
    if ((size_t *)heap_start != NULL)
    {
        return;
    }
    fprintf(stderr, "----- Initiliazing the heap -----\n");
    heap_start = init_block(8);
    fprintf(stderr, "break : %p\n", heap_start);
    size_t *heap_end = init_block(0);
    fprintf(stderr, "heap end : %p\n", heap_end);
    fprintf(stderr, "----- Heap initialized -----\n");
}

void *malloc(size_t size)
{
    void *free_block = NULL;
    size_t block_size = 0;

    init_heap();
    if (heap_start == NULL)
    {
        perror("Error when initializing heap");
        return NULL;
    }
    block_size = get_aligned_block_size(size);
    free_block = find_free_block(block_size);
    if (free_block == NULL)
    {
        return add_free_block(size);
    }
    return free_block;
}

// ENLEVER COMMENTAIRES, FAIRE FICHIERS POUR FONCTIONS (5 max par fichier)
// LD_PRELOAD=./libmalloc.so ls / -ltrR
// search test cases for memory allocator
// rendre fonctions statiques
// store the address of the first free block in prologue
// next point to payload
// gdb --args ls / -ltrR
// set environment LD_PRELOAD /home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so
#include "my_malloc.h"

extern size_t *heap_start;

// void *get_current_break(void)
// {
//     void *current_break = sbrk(0);

//     if (current_break == (void *)-1)
//     {
//         return NULL;
//     }
//     return current_break;
// }

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
    /*







    102 + 8 = 110
    payload + header = total size

    110/8 13.75 = 13
    13*8 = 104
    104%8
    104 + 8 = 112 bytes

    110%8 != 0 => +8

    13*8+8= 112 bytes

    padding = 112 - total_size 110 = 2 bytes

----------------------------
    malloc(2) : 8 + 2 = 10
    10/8 = 1.25 = 1
    8*1 = 8
    10%8 = 2

    8*1 + 8 = 16
----------------------------

    malloc(107)
    107/8 => 13.3 = 13
    8*13 = 104
    104%8
    104 + 8 = 112 bytes
   */
}

void *add_optional_padding(size_t size)
{
    void *optional_padding = sbrk(size);

    if (optional_padding == (void *)-1)
    {
        perror("Error when allocating optional padding");
        return NULL;
    }
    return optional_padding;
}

// size_t get_size(header_t *header)
// {
//     return *header & ~(1 << 0); // clear bit position 0; MAKE FUNCTION
// }

// int is_allocated(header_t *header)
// {
//     return CHECK_BIT(*header, 0);
// }

void *find_free_block(size_t size)
{
    // header_t *block_tmp_header = (header_t *)((size_t)heap_start - HEADER_SIZE);
    fprintf(stderr, "SHOULD START AFTER PROLOGUE HEADER\n");
    // fprintf(stderr, "Prologue header %p %ld\n", block_tmp_header, (size_t)block_tmp_header);
    // size_t size_tmp = get_size(block_tmp_header);
    // OLD WAY
    // header_t *block_tmp = heap_start; // (header_t *)((size_t)heap_start + size_tmp);
    size_t *block_tmp = NULL;
    header_t *block_tmp_header = NULL;
    size_t *block_tmp_prev = NULL;
    size_t *block_tmp_next = NULL;

    // size_t count = 0;
    size_t size_tmp = 0;

    if (*heap_start == 0)
    {
        return NULL;
    }
    block_tmp = (size_t *)(*heap_start);
    block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
    size_tmp = get_size(block_tmp_header);

    fprintf(stderr, "HEAD address %p payload %ld 0x%lx\n", heap_start, *heap_start, *heap_start);
    fprintf(stderr, "Block tmp %p %ld\n", block_tmp, (size_t)block_tmp);
    fprintf(stderr, "---- LOOP TO FIND FREE BLOCK ----\n");
    // OLD WAY
    // while (size_tmp != HEADER_SIZE)
    // {
    //     fprintf(stderr, "Current block address %p %ld | Header content: %ld %lb\n", block_tmp, (size_t)block_tmp, *block_tmp_header, *block_tmp_header);
    //     fprintf(stderr, "Current block size %ld %lb\n", size_tmp, size_tmp);
    //     fprintf(stderr, "Size to look for %ld | Current block allocated %d %b\n", size, is_allocated(block_tmp_header), is_allocated(block_tmp_header));
    //     if (size_tmp >= size && !is_allocated(block_tmp_header))
    //     { // if size_tmp <= size, split the block ?
    //         fprintf(stderr, "------- FOUND FREE BLOCK  -------\n");

    //         // fprintf(stderr, "Heap end header %p %ld %lb\n", block_tmp, *(header_t *)block_tmp, *(header_t *)block_tmp);
    //         // payload_start = (size_t *)((size_t)block_tmp + size_tmp);
    //         block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
    //         fprintf(stderr, "Payload start %p %ld | Header %p %ld %lb\n", block_tmp, (size_t)block_tmp, block_tmp_header, *block_tmp_header, *block_tmp_header);
    //         *(header_t *)block_tmp_header = *(header_t *)block_tmp_header | (1 << 0); // MAKE FUNCTION FOR SETTING AND CLEAR BIT
    //         return block_tmp;
    //         // break;
    //     }
    //     count += 1;
    //     block_tmp = (header_t *)((size_t)block_tmp + size_tmp);
    //     block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
    //     fprintf(stderr, "Increment current block\n"); //\nCurrent block address %p %ln | Header content : %ld %lb\n", block_tmp, block_tmp, *block_tmp_header, *block_tmp_header);
    //     size_tmp = get_size(block_tmp_header);
    //     // if (count == 5)
    //     // {
    //     //     fprintf(stderr, "Stop the car\n");
    //     //     break;
    //     // }
    // }

    while (block_tmp != NULL)
    {
        fprintf(stderr, "Current block address %p %ld | Header content: %ld %lb %p\n", block_tmp, (size_t)block_tmp, *block_tmp_header, *block_tmp_header, block_tmp_header);
        block_tmp_prev = (size_t *)(block_tmp + 1);
        block_tmp_next = (size_t *)((size_t)block_tmp);

        fprintf(stderr, "Prev %p next %p\n", block_tmp_prev, block_tmp_next);
        fprintf(stderr, "Values at prev %ld %lx next %ld %lx\n", *block_tmp_prev,  *block_tmp_prev, *block_tmp_next,  *block_tmp_next);

        // fprintf(stderr, "Current payload 0x%lx %ld next 0x%lx %ld prev %lx %ld\n", *block_tmp, *block_tmp, *block_tmp_next, *block_tmp_next, *block_tmp_prev, *block_tmp_prev);

        if (size_tmp >= size)
        { // if size_tmp <= size, split the block ?
            fprintf(stderr, "------- FOUND FREE BLOCK  -------\n");
            

            // fprintf(stderr, "Heap end header %p %ld %lb\n", block_tmp, *(header_t *)block_tmp, *(header_t *)block_tmp);
            // payload_start = (size_t *)((size_t)block_tmp + size_tmp);
            block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
            fprintf(stderr, "Payload start %p %ld | Header %p %ld %lb\n", block_tmp, (size_t)block_tmp, block_tmp_header, *block_tmp_header, *block_tmp_header);
            *(header_t *)block_tmp_header = *(header_t *)block_tmp_header | (1 << 0); // MAKE FUNCTION FOR SETTING AND CLEAR BIT

            *heap_start = *block_tmp_next;
            fprintf(stderr, "UPDATED HEAP START %ld %p\n", *heap_start, (size_t *)(*heap_start));
            fprintf(stderr, "BLOCK USED %p\n", block_tmp);
            return block_tmp;
        }
        block_tmp = (size_t *)(*block_tmp_next);
        block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
        fprintf(stderr, "Heyy %p\n", block_tmp);
    }
    return NULL;
    // fprintf(stderr, "Heap end header %p %ld %lb\n", block_tmp, *(header_t *)block_tmp, *(header_t *)block_tmp);
    // payload_start = block_tmp + HEADER_SIZE;
    // fprintf(stderr, "Payload start %p %ld %lb\n", payload_start, *(header_t *)payload_start, *(header_t *)payload_start);
    // return payload_start;
}

header_t *init_header(size_t size)
{
    header_t *header_start = sbrk(HEADER_SIZE);

    if (header_start == (void *)-1)
    {
        perror("Error when allocating header");
        return NULL;
    }
    // *(header_t *)header_start = size + HEADER_SIZE;
    *header_start = size;
    fprintf(stderr, "Header %p %ld %ld %lb\n", header_start, (size_t)header_start, *header_start, *header_start);
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
    // fprintf(stderr, "before allocating header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    *(header_t *)header_start = *(header_t *)header_start | (1 << 0); // set bit position 0
    // fprintf(stderr, "after allocating header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    return payload_start;
}

void *init_block(size_t payload_size)
{
    size_t block_size = get_aligned_block_size(payload_size);
    size_t optional_padding_size = block_size - (HEADER_SIZE + payload_size);
    header_t *header_start = init_header(block_size);
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
    fprintf(stderr, "In init block: padding start address %p %ld | Size %ld\n", optional_padding, (size_t)optional_padding, optional_padding_size);
    if (optional_padding == NULL)
    {
        return NULL;
    }
    return payload_start;
    // header_t *heap_end = init_block(0);
    // fprintf(stderr, "Before init heap end %p int %ld binary %lb\n", heap_end, *(header_t *)heap_end, *(header_t *)heap_end);
    // // heap_end = heap_start + HEAP_SIZE;
    // // *(header_t *)heap_end
    // // //  (*(unsigned int *)(p) = (val))
    // fprintf(stderr, "After init heap end %p int %ld binary %lb\n", heap_end, *(header_t *)heap_end, *(header_t *)heap_end);

    // void *header_start = sbrk(HEADER_SIZE);
    // void *payload_start = NULL;

    // if (header_start == (void *)-1)
    // {
    //     perror("Error when allocating header");
    //     return NULL;
    // }
    // *(header_t *)header_start = size + HEADER_SIZE;
    // fprintf(stderr, "Header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    // payload_start = sbrk(size);
    // if (payload_start == (void *)-1)
    // {
    //     perror("Error when allocating payload");
    //     return NULL;
    // }
    // fprintf(stderr, "before allocating header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    // *(header_t *)header_start = *(header_t *)header_start | (1 << 0);
    // fprintf(stderr, "after allocating header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    // return payload_start;
}

void *add_free_block(size_t payload_size)
{
    header_t *new_block_header_address = (header_t *)((size_t)get_current_break() - HEADER_SIZE);
    void *new_heap_end = NULL; // to comment
    void *new_block_payload = NULL;
    void *optional_padding = NULL; // to comment
    size_t block_size = 0;
    size_t optional_padding_size = 0;

    // payload_start = block_tmp + HEADER_SIZE;
    // fprintf(stderr, "Payload start %p %ld %lb\n", payload_start, *(header_t *)payload_start, *(header_t *)payload_start);
    // return payload_start;
    fprintf(stderr, "Old epilogue/new block header address %p %ld\n", new_block_header_address, (size_t)new_block_header_address);
    if (new_block_header_address != NULL)
    {
        block_size = get_aligned_block_size(payload_size);

        *new_block_header_address = block_size;
        new_block_payload = init_payload(payload_size, new_block_header_address);

        fprintf(stderr, "BLOCK SIZE IN ADD FREE BLOCK %ld payload %ld\n", block_size, payload_size);
        optional_padding_size = block_size - (HEADER_SIZE + payload_size);
        optional_padding = add_optional_padding(optional_padding_size);
        // OR
        // add_optional_padding(optional_padding_size);
        new_heap_end = init_block(0);
        // OR
        // init_block(0);
        fprintf(stderr, "New block address %p %ld\n", new_block_payload, (size_t)new_block_payload);
        fprintf(stderr, "New block address header %p %ld | Header %p %ld %lb\n", new_block_header_address, (size_t)new_block_header_address, new_block_header_address, *new_block_header_address, *new_block_header_address);
        fprintf(stderr, "New block padding start address %p %ld | Size %ld\n", optional_padding, (size_t)optional_padding, optional_padding_size);
        fprintf(stderr, "Heap end %p %ld | Header %p %ld %lb\n", new_heap_end, (size_t)new_heap_end, (header_t *)((size_t)new_heap_end - HEADER_SIZE), *(header_t *)((size_t)new_heap_end - HEADER_SIZE), *(header_t *)((size_t)new_heap_end - HEADER_SIZE));
        fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());
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
    fprintf(stderr, "--------- INITIALIZING HEAP ---------\n\n");
    fprintf(stderr, "---------  HEAP START (PROLOGUE BLOCK) --------- \n");
    fprintf(stderr, "===== BEFORE CREATING HEAP START =====\n");
    fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());

    // fprintf(stderr, "Before init heap start current break %p %ld\n", get_current_break(), (size_t)get_current_break());
    heap_start = init_block(8); // should be only a header and footer, fix later
    fprintf(stderr, "===== AFTER CREATING HEAP START =====\n");
    fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());
    fprintf(stderr, "Heap start address: %p %ld\n", heap_start, (size_t)heap_start);
    fprintf(stderr, "Heap start header address: %p %ld\n", (header_t *)((size_t)heap_start - HEADER_SIZE), (size_t)((size_t)heap_start - HEADER_SIZE));
    fprintf(stderr, "Heap start header content : %ld %lb\n", *(header_t *)((size_t)heap_start - HEADER_SIZE), *(header_t *)((size_t)heap_start - HEADER_SIZE));

    // fprintf(stderr, "After init heap start %p %ld int %ld binary %lb\n\n", heap_start, (size_t)heap_start, *(header_t *)heap_start, *(header_t *)heap_start);
    // fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());

    fprintf(stderr, "===== BEFORE CREATING HEAP END =====\n");
    fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());
    size_t *heap_end = init_block(0);
    // OR
    //  init_block(0);
    fprintf(stderr, "===== AFTER CREATING HEAP END =====\n");
    fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());
    fprintf(stderr, "Heap end address: %p %ld\n", heap_end, (size_t)heap_end);
    fprintf(stderr, "Heap end header content : %ld %lb\n", *(header_t *)((size_t)heap_end - HEADER_SIZE), *(header_t *)((size_t)heap_end - HEADER_SIZE));

    fprintf(stderr, "--------- END INITIALIZING HEAP ---------\n\n");

    // fprintf(stderr, "Before init heap end %p int %ld binary %lb\n", heap_end, *(header_t *)heap_end, *(header_t *)heap_end);
    // heap_end = heap_start + HEAP_SIZE;
    // *(header_t *)heap_end
    // //  (*(unsigned int *)(p) = (val))
    // fprintf(stderr, "After init heap end %p %ld int %ld binary %lb\n", heap_end, (size_t)heap_end, *(header_t *)heap_end, *(header_t *)heap_end);
}

void *malloc(size_t size)
{
    void *free_block = NULL;
    size_t block_size = 0;
    // if (free_block) {
    //     printf("ok %ld", size);
    // }

    init_heap();
    if (heap_start == NULL)
    {
        perror("Error when initializing heap");
        return NULL;
    }
    fprintf(stderr, "--------- FINDING FREE BLOCK ---------\n");
    fprintf(stderr, "Current break %p %ld\n", get_current_break(), (size_t)get_current_break());
    fprintf(stderr, "Heap start address: %p %ld\n", heap_start, (size_t)heap_start);
    fprintf(stderr, "Heap start header address: %p %ld\n", (header_t *)((size_t)heap_start - HEADER_SIZE), (size_t)((size_t)heap_start - HEADER_SIZE));
    fprintf(stderr, "Heap start header content : %ld %lb\n", *(header_t *)((size_t)heap_start - HEADER_SIZE), *(header_t *)((size_t)heap_start - HEADER_SIZE));

    // fprintf(stderr, "!!!!!!!!!!!!!!!Finding free block current break %p %ld heap start %p\n\n", get_current_break(), (size_t)get_current_break(), heap_start);
    block_size = get_aligned_block_size(size);
    fprintf(stderr, "BLOCK SIZE TO FIND %ld\n", block_size);
    free_block = find_free_block(block_size);
    if (free_block == NULL)
    {
        fprintf(stderr, "--------- ADDING FREE BLOCK ---------\n");
        return add_free_block(size);
    }
    return free_block;

    // search for free blocks before adding a new one

    // void *header_start = sbrk(HEADER_SIZE);
    // void *payload_start = NULL;

    // if (header_start == (void *)-1)
    // {
    //     perror("Error when allocating header");
    //     return NULL;
    // }
    // *(header_t *)header_start = size + HEADER_SIZE;
    // fprintf(stderr, "Header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    // payload_start = sbrk(size);
    // if (payload_start == (void *)-1)
    // {
    //     perror("Error when allocating payload");
    //     return NULL;
    // }
    // fprintf(stderr, "before allocating header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    // *(header_t *)header_start = *(header_t *)header_start | (1 << 0);
    // fprintf(stderr, "after allocating header %p %ld %lb\n", header_start, *(header_t *)header_start, *(header_t *)header_start);
    // return payload_start;
}

// ENLEVER COMMENTAIRES, FAIRE FICHIERS POUR FONCTIONS (5 max par fichier)
// LD_PRELOAD=./libmalloc.so ls / -ltrR
// search test cases for memory allocator
// rendre fonctions statiques
// store the address of the first free block in prologue
// next point to payload
#include "my_malloc.h"

extern size_t *heap_start;

size_t get_aligned_block_size(size_t payload_size)
{
    size_t total_size = HEADER_SIZE * 2 + payload_size;
    size_t div = total_size / ALIGNMENT_REQUIREMENT;
    size_t res = div * ALIGNMENT_REQUIREMENT;

    if (total_size % ALIGNMENT_REQUIREMENT != 0)
    {
        res += ALIGNMENT_REQUIREMENT;
    }
    return res;
}

size_t *add_optional_padding(size_t payload_size, void *start)
{
    // void *optional_padding = sbrk(size);

    // if (optional_padding == (void *)-1)
    // {
    //     perror("Error when allocating optional padding");
    //     return NULL;
    // }
    // // // ////fprintf((stderr, "\t[optional padding] %p (%ld) - %p (%ld)\n", optional_padding, (size_t)optional_padding, (size_t *)((size_t)optional_padding + size), ((size_t)optional_padding + size));
    // return optional_padding;
    return (size_t *)((size_t)start + payload_size);
}

void *find_free_block(size_t size)
{
    size_t *block_tmp = NULL;
    header_t *block_tmp_header = NULL;
    size_t *block_tmp_prev = NULL;
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
        block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
        size_tmp = get_size(block_tmp_header);
        block_tmp_prev = (size_t *)(block_tmp + 1);
        block_tmp_next = (size_t *)((size_t)block_tmp);

        if (size_tmp >= size)
        {
            block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
            *(header_t *)block_tmp_header = *(header_t *)block_tmp_header | (1 << 0); // MAKE FUNCTION FOR SETTING AND CLEAR BIT
            // ////fprintf((stderr, "[reused block] %p (%ld) - %p (%ld) : %ld bytes\n", block_tmp_header, (size_t)block_tmp_header, (size_t *)((size_t)block_tmp_header + size_tmp), (size_t)((size_t)block_tmp_header + size_tmp), size_tmp);
            
            *(size_t *)(*block_tmp_prev) = *block_tmp_next;
            if (*block_tmp_next != 0) {
                *(size_t *)(*block_tmp_next + 8) = *block_tmp_prev;
            }
            return block_tmp;
        }
        block_tmp = (size_t *)(*block_tmp_next);
    }
    return NULL;
}

header_t *init_boundary_tag(header_t *tag_start, size_t size)
{
    //fprintf((stderr, "TAG START %ld START %ld\n", (size_t)tag_start, (size_t)start);
    *tag_start = size;
    *tag_start = *(header_t *)tag_start | (1 << 0); // set bit position 0
    return tag_start;
    // // ////fprintf((stderr, "\t[header] %p (%ld) - %p (%ld): %ld %lb\n", tag_start, (size_t)tag_start, (size_t *)((size_t)tag_start + HEADER_SIZE), ((size_t)tag_start + HEADER_SIZE), *tag_start, *tag_start);
}

void *allocate_block(size_t size)
{
    void *block_start = sbrk(size);

    if (block_start == (void *)-1)
    {
        perror("Error when allocating header");
        return NULL;
    }
    return block_start;
}

void *get_payload_start(void *start)
{
    // // ////fprintf((stderr, "\t[payload] %p (%ld) - %p (%ld)\n", payload_start, (size_t)payload_start, (size_t *)((size_t)payload_start + size), ((size_t)payload_start + size));
    return start + HEADER_SIZE;
}

void *init_epilogue()
{
    void *block_start = allocate_block(HEADER_SIZE);

    if (block_start == NULL)
    {
        return NULL;
    }
    //fprintf((stderr, "    2Footer start %ld current break %ld\n", (size_t)block_start, (size_t)get_current_break());
    return init_boundary_tag(block_start, HEADER_SIZE);
    // ////fprintf((stderr, "[new block] %p (%ld) - %p (%ld): %d bytes\n", header_start, (size_t)header_start, (size_t *)((size_t)header_start + HEADER_SIZE), (size_t)header_start + HEADER_SIZE, HEADER_SIZE);
    // return header_start + HEADER_SIZE;
}

void *init_block(size_t payload_size)
{
    size_t block_size = get_aligned_block_size(payload_size);
    size_t optional_padding_size = block_size - (HEADER_SIZE * 2 + payload_size);
    header_t *block_start = allocate_block(block_size);
    header_t *header_start = NULL;
    header_t *footer_start = NULL;
    // size_t *block_end = NULL;
    void *payload_start = NULL;
    void *optional_padding = NULL;

    if (block_start == NULL)
    {
        return NULL;
    }
    //fprintf((stderr, "    Block start %ld\n", (size_t)block_start);
    header_start = init_boundary_tag(block_start, block_size);
    //fprintf((stderr, "    Header start %ld\n", (size_t)header_start);
    // block_end = (size_t *)((size_t)header_start + block_size);
    payload_start = get_payload_start(header_start);
    //fprintf((stderr, "    Payload start %ld\n", (size_t)payload_start);
    if (payload_start == NULL)
    {
        return NULL;
    }
    optional_padding = add_optional_padding(payload_size, payload_start);
    //fprintf((stderr, "    Optional padding %ld\n", (size_t)optional_padding);
    if (optional_padding == NULL)
    {
        return NULL;
    }
    footer_start = init_boundary_tag(optional_padding + optional_padding_size, block_size);
    //fprintf((stderr, "    Footer start %ld\n", (size_t)footer_start);
    if (footer_start == NULL)
    {
        return NULL;
    }
    //fprintf((stderr, "[new block] %p (%ld) - %p (%ld): %ld bytes\n", header_start, (size_t)header_start, block_end, (size_t)block_end, block_size);
    return payload_start;
}

void *add_free_block(size_t payload_size)
{
    header_t *new_block_header_address = (header_t *)((size_t)get_current_break() - HEADER_SIZE);
    // void *new_heap_end = NULL;
    void *new_block_payload = NULL;
    // size_t *block_end = NULL;
    size_t block_size = 0;
    size_t optional_padding_size = 0;
    void *optional_padding = NULL;
    // size_t *footer_start = NULL;
    // if (footer_start)
    // {
    // }

    if (new_block_header_address != NULL)
    {
        block_size = get_aligned_block_size(payload_size);
        // block_end = (size_t *)((size_t)new_block_header_address + block_size);
        //fprintf((stderr, "[heap end into new block] %p (%ld) - %p (%ld): %ld bytes\n", new_block_header_address, (size_t)new_block_header_address, block_end, (size_t)block_end, block_size);
        // if (block_end && new_heap_end)
        // {
        // };
        *new_block_header_address = block_size;
        *new_block_header_address = *(header_t *)new_block_header_address | (1 << 0); // set bit position 0
        new_block_payload = allocate_block(block_size - HEADER_SIZE);
        //fprintf((stderr, "-------NEW SIZE %ld NEW BLOCK HEADER ADDRESS %ld NEW BLOCK PAYLOAD %ld size %ld\n", block_size, (size_t)new_block_header_address, (size_t)new_block_payload, payload_size);
        // //fprintf((stderr, " Payload start %ld\n", (size_t)new_block_payload);

        optional_padding_size = block_size - (HEADER_SIZE * 2 + payload_size);
        optional_padding = add_optional_padding(payload_size, new_block_payload);
        //fprintf((stderr, "    Optional padding %ld size %ld\n", (size_t)optional_padding, optional_padding_size);
        // footer_start = 
        init_boundary_tag(optional_padding + optional_padding_size, block_size);
        //fprintf((stderr, "    Footer start %ld current break %ld\n", (size_t)footer_start, (size_t)get_current_break());
        init_epilogue();
        // ////fprintf((stderr, "heap end : %p\n", (size_t *)((size_t)new_heap_end - HEADER_SIZE));
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
    //fprintf((stderr, "----- Initiliazing the heap -----\n");
    heap_start = init_block(8);
    //fprintf((stderr, "break : %p\n", heap_start);
    size_t *heap_end = init_epilogue();
    if (heap_end)
    {
    };
    // ////fprintf((stderr, "heap end : %p\n", heap_end);
    // ////fprintf((stderr, "----- Heap initialized -----\n");
}

void *malloc(size_t size)
{
    void *free_block = NULL;
    size_t block_size = 0;
    // ////fprintf((stderr, "--------------------NEW MALLOC\n");
    init_heap();
    if (heap_start == NULL)
    {
        perror("Error when initializing heap");
        return NULL;
    }
    // //fprintf((stderr, "--------------------AFTER INITIATING HEAP\n");

    //check_valid_list();
    block_size = get_aligned_block_size(size);
    free_block = find_free_block(block_size);
    if (free_block == NULL)
    {
        return add_free_block(size);
    }
    return free_block;
}

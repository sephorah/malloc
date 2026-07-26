# malloc

This project involved rewriting `malloc`, `free`, `realloc` and `calloc` using `sbrk`.

## Features

- Reuse freed blocks using an explicit free list.
- Handle immediate coalescing by merging adjacent free blocks at each free.
- Insert newly freed blocks at the beginning of the free list (LIFO order).
- First-fit: the allocator uses the first block in the free list that fits.
- First-fit allocation time is linear in the number of free blocks.

<img width="933" height="560" alt="image" src="https://github.com/user-attachments/assets/020de70b-cf1a-42c7-9c2d-c48eed25412b" />

## Getting started

```
make
LD_PRELOAD=./libmalloc.so ls / -ltRr > /dev/null
```

## Known limitations

- Only `malloc`, `free`, `realloc` and `calloc` are implemented.
- First-fit: fast but causes more fragmentation.

## Performance


hyperfine --warmup 1 -r 1 -i  'ls / -ltRr' 'LD_PRELOAD=./libmalloc.so ls / -ltRr'

hyperfine --warmup 1 -r 10 \
  'make re' \
  'LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re'

/usr/bin/time -v make re
LD_PRELOAD=./libmalloc.so /usr/bin/time -v make re

valgrind --tool=callgrind --callgrind-out-file=cg.out \
  env LD_PRELOAD=./libmalloc.so g++ -std=c++17 -O2 -c one_file.cpp -o /dev/null
callgrind_annotate cg.out | head -30


make re CFLAGS="-Wall -Wextra -Werror -std=c++20 -O2"


<!-- `time LD_PRELOAD=./libmalloc.so ls / -ltRr > /dev/null` against the same command without the preload.

| Ratio | Run 1 | Run 2 | Run 3 |
|---|---|---|---|
| real | 1.05× | 1.03× | 1.03× |
| user | 1.20× | 1.25× | 1.26× |
| sys | 0.98× | 0.97× | 0.95× |

**`user` time:** +20-26% compared to glibc, probably because of first-fit which scans the free list linearly.

**`sys` time:** 0.95–0.98×. Before the free-list fix, the allocator made around 229k `brk` calls against glibc's 3k on `ls / -ltRr`. -->

<!-- Your numbers

glibc :   249 MB peak RSS,   563,746 minor faults
mine  :   754 MB peak RSS, 1,191,929 minor faults

You caused 2.1× as many faults and held 3.0× as much physical RAM. That's the no-splitting limitation, made physical: when a 16-byte request can't reuse an existing free block, you sbrk fresh address space instead. Then the program writes to it — fault, fault, fault — and each of those pages is now real RAM that stays resident. glibc would have carved that 16 bytes out of a block it already had, touching no new pages at all.

Those extra ~628,000 faults are also a large part of why your sys time was 3.22× on this workload. Each one is a trip into the kernel.

One thing that looks contradictory: 563,746 faults × 4096 bytes = 2.31 GB, but peak RSS was only 249 MB. Both are right. The build runs g++ 24 separate times, and each process faults in its own memory from scratch and then exits, releasing it. /usr/bin/time sums faults across all 24 children, but reports peak RSS as the highest single moment. Cumulative work vs. high-water mark. -->


## Tests

```
make tests_run
```

Unit tests on `malloc`, `free`, `realloc` and `calloc`.

## Examples of fixed bugs

### Unused free list

When walking the free list, the size wasn't refreshed on each iteration. Because of that, the allocator wasn't reusing freed blocks and kept calling `sbrk`, which significantly increased sys time.

**Fix:** Recompute header and size at each iteration in the free list.

Before the free-list fix, the allocator made around 229k `brk` calls against glibc's 3k on `ls / -ltRr`.

### Minimum block size

`git status` segfaulted while `ls / -ltRr` ran clean. The bug happened when the payload address of a block was equal to its footer address, which corrupted block merging.

A block with a 0-byte payload size has its payload and its footer at the same address. So, when the allocator added the block at the beginning of the free list (`add_block_free_list`), it was writing the `next` and `prev` pointers in the footer, which broke the `header == footer` invariant. I found it by calling the heap checker `check_heap` with `gdb` just after `add_block_free_list` returned.

**Fix:** set a minimum block size of 48 bytes, i.e. a payload of 16 bytes, so that every block's payload can hold `next` and `prev` and be added to the free list.

### Unlocked heap

If `malloc` or `realloc` is requesting to shrink a block, the allocator can return the same starting address and recycle the leftover space into another freed block.

This led to more unsynchronized work (update the found block and insert leftover). On multi-threaded commands like `git status`, this surfaced data corruption on the heap. For example, when walking the free list, a block's header (which is supposed to store a block's size) read as a heap address. However, when a watchpoint was set on this header on `gdb`, its last value was a size, meaning another thread modified it in the meantime.

**Fix:** set a mutex on the heap start so that only one thread can access the heap at a time.

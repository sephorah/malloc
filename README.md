# malloc

This project involved rewriting `malloc`, `free`, `realloc` and `calloc` using `sbrk`.

## Features

- Reuse freed blocks using an explicit free list.
- Handle coalescing by merging blocks at each free.
- Insert newly freed blocks at the beginning of the free list (LIFO order).
- First-fit: the allocator uses the first block in the free list that fits.
- First-fit allocation time is linear in the number of free blocks.

<img width="933" height="560" alt="image" src="https://github.com/user-attachments/assets/020de70b-cf1a-42c7-9c2d-c48eed25412b" />

## Getting started

```
make
LD_PRELOAD=./libmalloc.so ls / -ltRr > /dev/null
```

Works with ls, grep, cat, git commands, make and valgrind.

## Known limitations

- Single-threaded.
- Only malloc, free, realloc and calloc are implemented.
- First fit: fast but causes more fragmentation.
- No splitting: a 16-byte request can be served by a 4128-byte free block, and the leftover isn't reused.
- Internal helpers are still exported.

## Tests

```
make tests_run
```

Unit tests cover simple malloc/free, reuse of a freed block, `malloc(0)`, allocation larger than a page, 16-byte alignment of a returned pointer, double free, `free(NULL)`, and a churn sequence (20 mallocs, 20 frees, 20 mallocs) checked with `check_heap()` and `check_free_list()`.

## Fixed bugs

### Unused free list

The allocator wasn't reusing freed blocks and kept calling sbrk, which significantly increased sys time.
**Fix:** Recompute header and size at each iteration in the free list. Reduce the number of sbrk() when creating a block to decrease the sys time.

### Minimum block size

`git status` segfaulted while `ls -ltrR`, `cat` and `python3` ran clean.
The bug happened when the payload address of a block was equal to its footer address, which corrupted block merging.

A block with a 0-byte payload size has its payload and its footer at the same address. So, when the allocator wanted to add it at the beginning of the free list (`add_block_free_list`), it was writing the `next` and `prev` pointers in the footer, which broke the `header == footer` invariant. I found it by calling the heap checker `check_heap` with gdb just after `add_block_free_list`.

**Fix:** set a minimum block size of 48 bytes, i.e. a payload of 16 bytes, so that every block's payload can hold `next` and `prev` and be added to the free list.

### Header and footer not equal

After finding a free block, the header was marked allocated but not the footer.
**Fix:** mark both header and footer allocated.

### `calloc` missing

`calloc` wasn't implemented, which made `ls / -ltRr` crash (SIGSEGV in `merge_free_blocks` or "corrupted double-linked list"), although smaller `ls` commands worked fine.

`LD_PRELOAD` only overrides the symbols you define. `calloc` wasn't one of them, so those calls went to glibc. That meant that two allocators (the real one from glibc and mine) were using the heap at the same time but differently, which broke the heap.
**Fix:** implement and export `calloc`.

## Takeaways

- Checking the heap, the free list and all invariants would have helped me catch bugs sooner.
- Use `gdb` extensively, and other tools like strace and ltrace.
- Check whether the algorithm works as expected and the time it takes, not just the result.
- Be careful about the pointer arithmetic. Pointer ± n is scaled; integer ± n is raw bytes.

## Performance

`time LD_PRELOAD=./libmalloc.so ls / -ltRr > /dev/null` against the same command without
the preload, three runs of each (ratio = mine / glibc):

| Ratio | Run 1 | Run 2 | Run 3 |
|---|---|---|---|
| real | 1.05× | 1.03× | 1.03× |
| user | 1.20× | 1.25× | 1.26× |
| sys | 0.98× | 0.97× | 0.95× |

**`user` time:** +20-26% compared to glibc, probably because of first-fit which scans the free list linearly. I haven't isolated it yet.

**`sys` time:** 0.95–0.98×. Before the free-list fix, the allocator made around 229k `brk` calls against glibc's 3k on `ls / -ltRr`.
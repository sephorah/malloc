# malloc

This project involved rewriting `malloc`, `free`, `realloc`, `calloc` and `malloc_usable_size` using `sbrk`.

## Features

- Reuse freed blocks using an explicit free list.
- Handle immediate coalescing by merging adjacent free blocks at each free.
- Insert newly freed blocks at the beginning of the free list (LIFO order).
- First-fit: the allocator uses the first block in the free list that fits. Allocation time is linear in the number of free blocks.
- Thread-safe via a single mutex on the heap.

<img width="933" height="560" alt="image" src="https://github.com/user-attachments/assets/020de70b-cf1a-42c7-9c2d-c48eed25412b" />

## Getting started

```
make
LD_PRELOAD=./libmalloc.so ls / -ltRr
```

## Known limitations

- Unlike `glibc`'s [malloc](https://sourceware.org/glibc/wiki/MallocInternals), every allocation comes from `sbrk`.
- Only `malloc`, `free`, `realloc`, `calloc` and `malloc_usable_size` are implemented.
- First-fit: fast but causes more fragmentation.
- Memory is never returned to the OS: the allocator should return to the OS all freed blocks that sit between the last allocated block and the current break.

## Comparison with `glibc`'s malloc

An old [C++ school project](https://github.com/sephorah/nanotekspice) is used for this section.

### On a real program

[Hyperfine](https://github.com/sharkdp/hyperfine) is a command-line benchmarking tool.

```
hyperfine --warmup 1 -r 5 'make re' 'LD_PRELOAD=path/to/libmalloc.so make re'
```

| | real | user | sys |
|---|---|---|---|
| glibc | 11.668 s ±  0.345 s | 10.665 s | 0.921 s |
| this allocator | 13.363 s ±  0.292 s | 12.217 s | 1.057 s |

Across 5 runs, `make re` ran 1.15-1.25 times faster than `LD_PRELOAD=path/to/libmalloc.so make re`. It's likely explained by the first-fit which scans the free list linearly, unlike `glibc`'s malloc which can find suitable free chunks in bins, various lists based on size and history.

### Memory cost

`/usr/bin/time` is a tool used to measure the runtime and memory usage of a given program.

```
/usr/bin/time -v make re 2>&1 | grep -E 'Maximum resident|Minor|Major'
/usr/bin/time -v env LD_PRELOAD=path/to/libmalloc.so make re 2>&1 | grep -E 'Maximum resident|Minor|Major'
```

| | glibc | this allocator | difference
|---|---|---|---|
| Maximum resident set size (kbytes) | 229312 | 257296 | +12.2% |
| Major (requiring I/O) page faults | 0 | 0 | none |
| Minor (reclaiming a frame) page faults | 588874 | 608822 | +3.4% |

## Tests

```
make tests_run
```

Unit tests on `malloc`, `free`, `realloc`, `calloc` and `malloc_usable_size`.

## Examples of fixed bugs

### Unused free list

When walking the free list, the header and size of the current block weren't updated on each iteration. Because of that, the allocator wasn't reusing freed blocks and kept calling `sbrk`, which significantly increased sys time.

**Fix:** Recompute header and size at each iteration in the free list.

Before the free-list fix, the allocator made around 229k `brk` calls against glibc's 3k on `ls / -ltRr`.

### Minimum block size

`git status` segfaulted while `ls / -ltRr` ran clean. The bug happened when the payload address of a block was equal to its footer address, which corrupted block merging.

A block with a 0-byte payload size has its payload and its footer at the same address. So, when the allocator added the block at the beginning of the free list (`add_block_free_list`), it was writing the `next` and `prev` pointers in the footer, which broke the `header == footer` invariant. I found it by calling the heap checker `check_heap` with `gdb` just after `add_block_free_list` returned.

**Fix:** set a minimum block size of 48 bytes, i.e. a payload of 16 bytes, so that every block's payload can hold `next` and `prev` and be added to the free list.

### Data race on the free list

When `realloc` shrinks a block, or when `malloc` serves a request from a free block larger than needed, the allocator can return the same starting address and recycle the leftover space into another free block.

This led to more unsynchronized work (updating the found block and inserting the leftover). On multi-threaded commands like `git status`, this surfaced data corruption in the heap. For example, when walking the free list, a block's header (which is supposed to store a block's size) read as a heap address. However, when a watchpoint was set on this header in `gdb`, its last value was a size, which probably meant that another thread modified it in the meantime.

**Fix:** setting a mutex on the heap start, so that only one thread can access the heap at a time. After that, the bug was no longer reproducible.

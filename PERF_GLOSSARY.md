# Performance & Memory Glossary

Reference notes for the Performance section. Definitions only.

## General

**Overhead** — resources consumed by the *mechanism* rather than by the work it exists to do. Always relative to a hypothetical zero-cost version of the same job.

- **Space overhead** here: 32 bytes of boundary tags on every block, plus alignment padding. A `malloc(1)` costs 48 bytes.
- **Time overhead** here: the free-list walk, the mutex lock/unlock pair, the `sbrk` trap.

"+14.6% CPU vs glibc" is a statement about overhead: same work done, more resources spent doing it.

**Word** — the natural unit of data a given processor handles: the width of its general-purpose registers, and of a single register-sized load or store. **It has no fixed size** — it is a property of the architecture, not of C. 64 bits on x86-64, 32 on ARMv7, 16 on a classic 8086.

Two traps in the terminology:

- x86 *assembly mnemonics* freeze the historical 8086 meaning: `word` = 16 bits, `dword` = 32, `qword` = 64. So "word" in an x86 instruction name means something different from "machine word".
- The C standard never mentions words. `sizeof(int)` is 4 even on 64-bit machines; the type that tracks the word size is `size_t` / `void *` (8 bytes here).

Why it matters for an allocator: a load crossing a word boundary is slower or illegal depending on the CPU, so the ABI mandates alignment. This allocator aligns to **16 bytes = two words**, which is what x86-64 requires for `long double` and SSE registers.

## Time

| Term | Meaning |
|------|---------|
| **real** (wall/elapsed) | Stopwatch: start to finish. What a user perceives. |
| **user** | CPU cycles spent executing *your* code, in user mode (ring 3). |
| **sys** | CPU cycles spent executing *kernel* code **on behalf of your process** (ring 0). |

**`real = user + sys + blocked time`** (single-threaded).

- `real ≠ user + sys`. The gap is time the process existed but was **not running**: waiting on disk, waiting on a lock, or descheduled while other processes ran.
- **sys is not "time waiting for the kernel."** If the kernel blocks on disk, the process is off-CPU entirely — that time is counted in *neither* user nor sys. It only appears in real.
- Multithreaded programs can have `user + sys` **exceed** real (4 cores × 1s = up to 4 CPU-seconds).

**CPU cycle** — one tick of the processor clock, the basic unit of CPU time. At 3 GHz, one cycle ≈ 0.33 ns. Instructions take one or more cycles. user/sys times are really cycle counts attributed to the process.

## Memory

**RSS (Resident Set Size)** — how much **physical RAM** the process actually occupies right now, as opposed to virtual address space it has merely reserved. `/usr/bin/time -v` reports the peak (high-water mark).

**Page fault** — the process touches a virtual page not currently backed by a physical page. The MMU can't translate the address, so the hardware raises an exception and the kernel's handler runs, maps a page, then **re-executes the faulting instruction**. The program never notices.

- **minor** — no disk needed; the kernel just supplies a (usually zeroed) page. This is what a freshly-grown heap produces on first touch.
- **major** — requires disk I/O (swap or file-backed). Expensive. Should be ~zero for a normal run.

**Lazy allocation (demand paging)** — "lazily" = deferred until actually needed. `sbrk` only extends the *virtual* address range; the kernel does **not** hand over physical page frames until the memory is first **touched**. So virtual size grows instantly and cheaply, while RSS grows gradually, one minor fault per page, as the program writes.

**Swap** — disk space the kernel uses as an overflow area for physical RAM. Under memory pressure it evicts pages to swap and reuses their frames; touching an evicted page later causes a **major** page fault, which must wait on disk. Swap is orders of magnitude slower than RAM, so a healthy benchmark run has essentially **zero** major faults — which is what the measurements show (0 vs 2).

(The same word also names the unrelated act of exchanging two values, as in "swap the pointers". Context disambiguates.)

**Fragmentation** — memory that is free in total but not usable in the shape requested. Two distinct kinds:

| | Where the waste is | Cause here |
|---|---|---|
| **External** | *between* blocks | free space is split into pieces separated by live blocks, so no single piece is big enough even though the sum is |
| **Internal** | *inside* a block | bytes handed to the caller but unusable: boundary tags, alignment padding, rounding up to `MIN_BLOCK_SIZE` |

External fragmentation is what **coalescing** fights: merging adjacent free blocks turns many small pieces back into one large one. **First-fit** makes it worse than best-fit — it carves the first block that fits, which tends to shave large blocks into unusable slivers near the head of the list.

Internal fragmentation is structural and unavoidable in a boundary-tag design: `malloc(1)` consumes 48 bytes, so 47 are internal fragmentation.

A general-purpose allocator cannot fix external fragmentation by moving blocks — it handed out raw pointers and has no idea who holds them. Only a system that can rewrite references (a garbage collector) can compact.

## Kernel interaction

**I/O** — data movement between the process and the outside world (disk, network, terminal, pipes). Two properties: it's orders of magnitude slower than CPU/RAM (cache hit ~1 ns, RAM ~100 ns, SSD read ~100,000 ns), and it goes through the kernel, usually **blocking** the process.

**Trap** — a hardware-level control transfer that switches privilege level (ring 3 → ring 0), saves registers, and jumps to a fixed kernel entry point registered at boot. The process cannot choose where it lands; that is what makes it safe.

- **Deliberate trap** — the `syscall` instruction. What `sbrk` compiles to.
- **Involuntary trap** — a page fault, interrupt, or fault such as divide-by-zero.

### The hardware mechanism

It is the **CPU**, not the kernel, that performs the transfer. On x86-64 the kernel registers an **IDT** (Interrupt Descriptor Table) at boot, mapping each vector number to a handler address, plus an `MSR` holding the `syscall` entry point. When a trap fires the hardware, in one indivisible step:

1. switches to ring 0,
2. switches to the kernel stack for this CPU,
3. pushes the interrupted state (instruction pointer, flags, and for a page fault the faulting address in `CR2`),
4. jumps to the registered handler.

User code cannot alter steps 1–4 or pick a different destination. That is the whole basis of kernel/user isolation: untrusted code is *allowed* to trigger traps precisely because it cannot influence where they land.

Three classes, in x86 terminology:

| Class | Restartable? | Resumes at | Example |
|---|---|---|---|
| **Fault** | yes | the **same** instruction, re-executed | page fault, `#GP` |
| **Trap** (narrow sense) | yes | the **next** instruction | `syscall`, breakpoint `int3` |
| **Abort** | no | — | machine-check, unrecoverable hardware error |

The fault/trap distinction is what makes demand paging invisible: the handler maps a page, returns, and the *same* load runs again and now succeeds. The program never learns it happened.

Also worth separating: an **interrupt** is *asynchronous* — raised by external hardware (timer, network card) with no relation to the current instruction. A **trap/exception** is *synchronous* — caused by the instruction being executed.

**Cost** — the mode switch alone is hundreds to a few thousand cycles, before the handler does any work, and it flushes pipeline state and pollutes cache/TLB. This is the entire reason `malloc` exists as a user-space layer: one `sbrk` trap is amortized over thousands of allocations that cost no trap at all.

## Threads and hardware topology

**Thread** — an independent stream of execution inside a process. Threads of the same process **share the address space**: heap, globals, file descriptors, code. Each has its own **stack, registers, and program counter**.

That sharing is the whole reason an allocator needs locking. `heap_start`, the free list, and every boundary tag are shared mutable state, and on a multi-core machine two threads can be inside `malloc` at the *same instant on different cores* — not interleaved, genuinely simultaneous. Hence the mutex around `heap_start`.

Note "thread" is also used for **hardware threads** (SMT / hyper-threading): two logical CPUs sharing one physical core's execution units. `lscpu` on this machine reports 4 cores × 2 threads = 8 logical CPUs.

**Thread locality** — keeping the data a thread uses private to that thread, so threads do not compete for the same memory. Two independent payoffs:

*1. Lock contention.* Data that isn't shared needs no lock. glibc gives each thread its own **arena** — a separate heap region with its own lock — so most `malloc` calls never touch a contended lock, and threads scale. This allocator has **one global mutex**, so every thread serializes on it: with N threads the allocator is effectively single-threaded. That is the direct, measurable cost of having no thread locality. It is a correctness-first choice, not a performance one.

*2. Cache behaviour and false sharing.* Each core has private L1/L2 caches, and coherence works at **cache-line** granularity — 64 bytes here. When one core writes a line, that line must be invalidated in every other core's cache. So two threads writing to **different variables that happen to sit in the same 64-byte line** still bounce the line between cores at full coherence cost, despite never sharing data. This is **false sharing**, and the fix is padding hot per-thread data out to a full cache line.

**NUMA (Non-Uniform Memory Access)** — on multi-socket machines, each socket has its own memory controller and its own bank of RAM. A core reading its **local** bank is fast; reading another socket's bank crosses an interconnect, costing noticeably more latency and less bandwidth. "Non-uniform" = *the cost of a memory access depends on which core issues it and where the page physically lives.*

- Linux places a page on the node of the thread that **first touches** it (**first-touch policy**). So where memory is *initialized* matters as much as where it is used — a common bug is one thread initializing a large buffer that many threads then use, parking all of it on one node.
- Allocator consequence: a block freed by a thread on node 0 and recycled by a thread on node 1 keeps its original physical placement. Reuse is correct but silently remote. NUMA-aware allocators keep per-node free lists.
- `numactl --hardware` or `lscpu` shows the layout. **This machine is single-socket, 1 NUMA node** (verified), so NUMA is irrelevant to these measurements — it only matters on servers.

## Heap syscalls

### `malloc` vs `mmap` — different layers, not alternatives

| | `mmap` | `malloc` |
|---|---|---|
| What it is | a **syscall** — kernel service | a **library function** — user-space code |
| Costs a trap? | yes, every call | no (only when it must refill) |
| Granularity | **whole pages** (4 KB), page-aligned | bytes (subject to alignment) |
| Implemented by | the kernel | libc — or us |

`malloc` is **built on top of** `mmap`/`sbrk`. Its job is *amortization*: ask the kernel for large chunks rarely, then hand out small pieces cheaply from that pool without trapping.

Why not `mmap` every allocation? `malloc(10)` would burn a 4096-byte page; every allocation would become a syscall; and each mapping costs the kernel a VMA structure, so thousands degrade address-space operations process-wide.

`mmap` is not "the large-allocation API" — it is a general virtual-memory mechanism (it also maps files, creates shared memory, and loads shared libraries). glibc merely *chooses* it above a size threshold.

### The calls

**`sbrk(n)`** — moves the "program break" by `n` bytes and returns the **previous** break. The break is the address **one past the end** of the heap — the first byte *not* in it. `sbrk(0)` returns it without changing it. Forward = grow, backward = shrink. Because it is one linear region, it can only be grown or shrunk **at the top**.

**`mmap`** — creates a *new, independent* region of virtual address space (anonymous memory or a file mapping), separate from the `brk` heap.

**`munmap`** — the inverse: destroys a mapping and returns its physical pages to the OS immediately. Because `mmap` regions are independent of each other, they can be released **in any order** — not only from the top.

**glibc's behaviour** (for contrast):

- Allocations above `MMAP_THRESHOLD` (default ~128 KB) get their own `mmap` region, `munmap`'d on `free`.
- The main heap is trimmed back down with `brk` when the top free block exceeds `M_TRIM_THRESHOLD` (default ~128 KB).

## Returning memory to the OS

**The rule: only the free block whose end touches the break can ever be returned.**

`brk` exposes a **single movable boundary**. Moving it backward releases the range `[new_break, old_break)` — always the **topmost** bytes. There is no way to punch a hole in the middle.

```
 0x1000       0x1100       0x1200       0x1300       0x1400
   |  BLOCK1   |  BLOCK2    |  BLOCK3    |  BLOCK4    |
   | allocated |   FREE     | allocated  |   FREE     | <- break
                             ^ a live pointer p == 0x1200
```

- Return `BLOCK4`: `sbrk(-0x100)` releases exactly `[0x1300, 0x1400)`. Free, unreferenced. **Safe.**
- Return `BLOCK2`: the break must travel down to `0x1100`, releasing `[0x1100, 0x1400)` — which includes the **live** `BLOCK3` at `p`. Segfault, or silent corruption once the kernel reuses those pages. **Impossible.**

**Reuse is a separate question and is unaffected.** Interior free blocks stay on the free list and still serve future `malloc`s; size-vs-request governs reuse, position governs releasability. Above, all four blocks behave normally for reuse; only *one* was ever a shrink candidate.

Because free blocks are coalesced, the candidate is always a single block, testable with `payload + size == sbrk(0)`.

`mmap` sidesteps this entirely: independent regions can be `munmap`'d **regardless of position** — the stack-of-plates vs. plates-on-a-table difference. That is why glibc routes large allocations there.

### Why not trim immediately

Cost of releasing a 128 KB block and needing it again right away:

| | Trim immediately | Keep it |
|---|---|---|
| `sbrk(-n)` | 1 syscall | 0 |
| `sbrk(+n)` | 1 syscall | 0 |
| minor faults | **32** (4 KB pages) | 0 |
| kernel zeroing | 128 KB | 0 |
| cache state | cold | warm |

Reclaimed frames are gone, so each page must fault back in, and the kernel must **zero every page** before handing it over (it cannot leak another process's data). A program allocating and freeing around that size would pay this on *every* cycle — hence a threshold, i.e. hysteresis, rather than "always" or "never".

## Tools — three questions, three tools

| Question | Tool | Metric |
|---|---|---|
| End-to-end cost in a real program | `hyperfine` | real, user, sys + σ |
| What did it cost in memory? | `/usr/bin/time -v` | peak RSS, minor/major faults |
| Where does the time go? | `valgrind --tool=callgrind` | instruction counts per function |

- `hyperfine` measures **wall clock** with warmup and repetition, so it is statistically sound where a single `time` run is not. It is the right tool for "does swapping the allocator change end-to-end runtime?" — and the *wrong* tool for "how fast is my allocator?" on a syscall-bound workload.
- `/usr/bin/time` (not the bash builtin `time`, which has no `-v`).
- `callgrind` counts instructions **deterministically** — no timing noise — but runs 10–50× slower and ignores the wall-cost of syscalls.

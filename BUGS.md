# Bugs & Fixes

Each bug: **symptom → fix**. Fill in **Why** yourself, then check against the hints.

Doesnt work with git push, pnpm dev 
xrecallocarray: out of memory (0 elements of 48 bytes), curl --version, 

---

## 1. `old_block_prev` pointer arithmetic — `free.c` (commit `4de75a2`)
- **Symptom:** free list corrupted after a few frees; back-pointers pointed at the wrong place.
- **Fix:**
```c
- old_block_prev = (size_t *)(*heap_start + 1);   // before: +1 byte → 7 bytes short of the prev slot
+ old_block_prev = (size_t *)(*heap_start + 8);   // after:  +8 bytes → skip next-ptr, land on prev slot
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
A freed block stores two pointers: `ptr[0]` = next, `ptr[1]` = the address of the slot that points *to this block*. The prev slot lives 8 bytes past the payload start (`+8`). It must point to a *pointer* (either `heap_start` or the previous node's `ptr[0]`), so removing a node mid-list can fix up the link without walking from the head.
</details>

---

## 2. Free-list walk — `find_free_block` (commit `f67e007`)
- **Symptom:** allocator rarely reused freed blocks; kept calling `sbrk` (huge `brk` count under `strace -c`).
- **Fix:** recompute header/size *each* iteration, track prev, and unlink via both links (doubly-linked), not just the head.
```c
  while (block_tmp != NULL) {
-     // block_tmp_prev = (size_t *)(block_tmp + 1);   // before: prev disabled; size_tmp stale (set once, outside loop)
+     block_tmp_header = (header_t *)((size_t)block_tmp - HEADER_SIZE);
+     size_tmp = get_size(block_tmp_header);            // after: refresh size for THIS node
+     block_tmp_prev = (size_t *)(block_tmp + 1);
      block_tmp_next = (size_t *)((size_t)block_tmp);
      if (size_tmp >= size) {
          ...
-         *heap_start = *block_tmp_next;                // before: only fixes the HEAD
+         *(size_t *)(*block_tmp_prev) = *block_tmp_next;        // after: unlink anywhere in the list
+         if (*block_tmp_next != 0)
+             *(size_t *)(*block_tmp_next + 8) = *block_tmp_prev;
          return block_tmp;
      }
      block_tmp = (size_t *)(*block_tmp_next);
  }
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
The walk has to follow `ptr[0]` (next payload) each step and stop at 0 (tail). Following the wrong field, or not advancing, means the search inspects one node and gives up → no reuse → fresh `sbrk` every time.
</details>

---

## 3. Missing `calloc` — `calloc.c` (this session, the big one)
- **Symptom:** `ls / -ltrR` crashed nondeterministically — SIGSEGV in `merge_free_blocks` OR "corrupted double-linked list" abort. Smaller commands often ran fine.
- **Fix:** implement and export `calloc` (route through your `malloc` + `memset`).
```c
// before: src/calloc.c did not exist → `nm -D libmalloc.so` showed NO calloc symbol
+ void *calloc(size_t nmemb, size_t size) {       // after: now overridden by us
+     size_t total;
+     if (__builtin_mul_overflow(nmemb, size, &total)) return NULL;
+     void *p = malloc(total);
+     if (p) memset(p, 0, total);
+     return p;
+ }
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
LD_PRELOAD only overrides the symbols you define. If you don't export `calloc`, the program's calloc calls go to **glibc's** allocator — which carves the **same brk heap** you do (both use `sbrk`), but in its own format. Now two allocators interleave blocks on one heap → glibc's metadata reaches your `free`, your `ptr-16` lands on a foreign header (e.g. a libgcc pointer), and you crash. An LD_PRELOAD allocator must override **every** allocation entry point. Verify with `nm -D`.
</details>

---

## 4. `calloc` memset length — `calloc.c:19` (this session)
- **Symptom:** systemd's `json.c` assertion `*q > 0` aborted — a **logic** assert about a value, not a segfault.
- **Fix:**
```c
- memset(memory, 0, nmemb);   // before: zeroes only nmemb bytes → tail left as garbage
+ memset(memory, 0, total);   // after:  zeroes the whole nmemb*size block
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
calloc must zero the **whole** block (`nmemb * size = total`), not `nmemb` bytes. Why only some programs noticed: a fresh `sbrk` page is already kernel-zeroed, so the missing memset is invisible there; a **recycled free-list block** holds old data, so the un-zeroed tail is garbage. A program that calloc'd a struct and trusted a field to be 0 read garbage → its own assertion fired. (It's a logic assert, not a segfault, because the memory was valid — just not zeroed.)
</details>

---

## 5. `find_free_block` didn't mark the FOOTER allocated — `malloc.c` (uncommitted)
- **Symptom:** coalescing later mis-merged a live block; header and footer disagreed.
- **Fix:** on a free-list hit, mark BOTH header and footer allocated.
```c
  *(boundary_tag_t *)block_tmp_header = SET_BIT(block_tmp_header, 0);   // before: header only
+ *(boundary_tag_t *)block_tmp_footer = SET_BIT(block_tmp_footer, 0);   // after:  footer too
// (also added: block_tmp_footer = (boundary_tag_t *)((size_t)block_tmp + size_tmp - BOUNDARY_TAG_SIZE*2);)
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
The boundary-tag invariant is header == footer. If you flip the alloc bit only in the header when reusing a block, the footer still reads "free". The next call to `free` on a neighbor inspects that stale footer, thinks this live block is free, and coalesces it away → corruption.
</details>

---

## 6. `realloc` `check_available_space` compared total size vs payload — `realloc.c` (uncommitted)
- **Symptom:** realloc returned the same block when it was actually too small; caller overran it.
- **Fix:** compare the *payload* capacity, not the *total* block size.
```c
- size_t block_size = get_size(header);                       // before: total = payload + 32
- if (block_size >= size)
+ size_t payload_size = get_size(header) - BOUNDARY_TAG_SIZE * 2;   // after: subtract the 2 tags
+ if (payload_size >= size)
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
`get_size` returns the **total** block size = payload + 2 tags (+32). Comparing that to the requested *payload* overcounts by 32, so a block 20 bytes too small looks like it fits → realloc hands back a block the caller then writes past.
</details>

---

## 7. `realloc` `memcpy` over-read the old block — `realloc.c` (uncommitted)
- **Symptom:** reading garbage / past the old allocation during grow-realloc.
- **Fix:** copy `min(old_payload, size)`, and guard `malloc == NULL`.
```c
  new_block = malloc(size);
- memcpy(new_block, ptr, size);                                  // before: copies NEW size from OLD (smaller) block
+ if (new_block == NULL) return ptr;                             // after:  guard NULL
+ old_block_payload_size = get_old_block_payload_size(ptr, size);
+ memcpy(new_block, ptr, old_block_payload_size);                // after:  copy only what the old block holds
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
`memcpy(new, ptr, size)` copies the **new (larger)** size out of the **old (smaller)** block → reads past the old block's payload into neighboring memory. You may only copy what the old block actually holds: `min(old_payload, new_size)`. And if `malloc` returns NULL you must not memcpy into it.
</details>

---

## 8. `merge_free_blocks` rewrite — extracted to `src/merge_free_blocks.c` (uncommitted)
- **Symptom:** the old in-`free.c` coalescing (`check_next_block` / `update_next_neighbor`) mismanaged the free-list links when merging.
- **Fix:** deleted the old version; reimplemented coalescing in `src/merge_free_blocks.c`.
```c
// before (free.c): merge updated sizes but left list links dangling, e.g.
-   *next_block_footer = get_size(next_block_header) + get_size(current_block_header);
-   *current_block_header = *next_block_footer;
-   update_next_neighbor(current_block_header);   // ad-hoc link fixup, buggy
-   *next_block_header = 0;
// after: coalescing lives in src/merge_free_blocks.c; header==footer kept in sync
//        and the merged-away node is properly unlinked from the explicit free list.
```

**Why this fix was needed:**
> _(your answer here — describe what correct prev/next coalescing must preserve)_

<details><summary>hint</summary>
When you merge a block with a neighbor, the merged result must (a) update BOTH header and footer to the combined size, and (b) keep the explicit free list consistent — the merged-away node must be unlinked and back-pointers fixed. The old version updated sizes but left dangling list links.
</details>

---

## 9. Debug `// fprintf` inside the allocator recursed into `malloc` — `malloc.c` / `realloc.c` / `merge_free_blocks.c` (this session, splitting work)
- **Symptom:** while adding block splitting, `traverse_free_list` read a block's size into `size_tmp` (line 46) but by the time it *used* it (line 50) the value was a heap address instead of a size → segfault in `handle_free_block_found`. In gdb, same run, same block: `*block_tmp_header == 65` (a valid size-64 *allocated* header) yet the local `size_tmp` held `0x5555...` — the memory changed **between** the read and the use, with only a debug `// fprintf` (line 48) in between.
- **Fix:** take `// fprintf` out of the allocator hot paths. If logging is needed inside `malloc`/`free`/`realloc`/coalescing, `snprintf` into a stack buffer and `write(2)` to stderr (no allocation), or gate the log behind a reentrancy flag — so logging can never call back into the allocator.
```c
// before: inside traverse_free_list, between reading and using the block
- // fprintf(stderr, "In traverse free list header %ld size %ld\n", (size_t)block_tmp_header, size_tmp);
// after: no // fprintf in the allocator; if needed:
+ char buf[128]; int n = snprintf(buf, sizeof buf, "...\n", ...); write(2, buf, n);
```

**Why this fix was needed:**
> _(your answer here)_

<details><summary>hint</summary>
`// fprintf` can call `malloc` internally (buffering, wide-char/locale, float, wide fields). Inside your own `malloc`, that **re-enters** the allocator: first-fit grabs the very free block you're mid-traversal on, `mark_boundary_tag_allocated` flips its header (64 → `65`) and removes/splits it, then hands its payload to `// fprintf`. Control returns to your outer loop, which now uses a **stale** `size_tmp`/`block_tmp` for a block that is already live → corruption. Two conditions had to coincide: (1) the print actually reached `malloc` (not every `// fprintf` does — that's why earlier prints "worked"), and (2) splitting introduced a read→use gap with the print sitting in it, on a live free-list block. A print inside an allocator is only safe if it cannot recurse into the allocator **and** nothing after it depends on state the recursion could change. The Heisenbug tell: the value looked fine in a later gdb peek because by then the reentrant allocation had settled.
</details>

---

## Known limitations (documented, not fixed)
  750  LD_PRELOAD=./libmalloc.so grep -rn "malloc" src/
  751  LD_PRELOAD=./libmalloc.so grep    "malloc" src/malloc.c

### Concurrency race — allocator is single-threaded / unlocked
- **Symptom:** intermittent SIGSEGV (exit 139) under LD_PRELOAD into *multithreaded*
  programs — e.g. `grep -rn "malloc" src/` (grep is a shell function wrapping
  multithreaded **ugrep**), `make -j`, `git status` with `core.preloadindex`.
  The *same command* usually runs clean and crashes only occasionally.
- **Mechanism:** `heap_start` and every explicit-free-list link are read/written
  with **no lock or atomics** (`grep -rnE "pthread|mutex|atomic" src/` → nothing).
  Two threads in `malloc`/`free` at once interleave their edits to the shared free
  list and corrupt it. Intermittent because most thread interleavings happen not to
  collide — which is exactly why a single-file `grep src/malloc.c` (one file → ugrep
  stays single-threaded) never reproduces it, while the recursive form sometimes does.
- **Why it crashed in the Claude Code session but "worked" in my own terminal:**
  `grep` is not the same program in both places. In the Claude Code shell, `grep`
  is a shell **function** (`type grep`) that runs the **multithreaded `claude`
  binary** in ugrep-emulation mode as the search engine — a very thread-heavy
  client → concurrent malloc/free → crashes **5/5, deterministically**. In my
  plain vscode/bash terminal, `grep` is single-threaded **`/usr/bin/grep`** → one
  caller, no contention → clean every time. Same files, same allocator, same bug —
  the only variable was which program the word "grep" launches. (This is also why
  it looked like the Claude session "had different files" — it didn't; it had a
  different `grep`.)
- **Reproducer:** `tests/thread_safety.c` — 8 threads × 20k malloc/free ops.
  Removes the luck: crashes ~every run.
```
cc tests/thread_safety.c -pthread -o thread_test
LD_PRELOAD=./libmalloc.so ./thread_test     # SIGSEGV (exit 139), 10/10 runs
./thread_test                               # WITHOUT preload: "survived", 0/10 crashes
```
  The no-preload control proves the test itself is race-free; the corruption is in
  the allocator.
- **Confirmed again via `git status` + a single-thread control (this session).**
  After implementing block splitting, `LD_PRELOAD=./libmalloc.so git status` began
  to SIGSEGV — even though it ran clean before splitting. The clean control:
```
LD_PRELOAD=./libmalloc.so git -c core.preloadindex=false status   # runs clean, every time
```
  `git status` parallelizes its index scan with `core.preloadindex` (~one thread per
  core). Forcing it single-threaded removes the crash → the fault is the **race**, not
  a splitting bug. (Splitting itself is correct; the single-threaded path stays healthy.)
- **Why it was dormant before splitting, then fired.** The race was always present
  (no locks). A data race only corrupts when a second thread touches the shared
  structure during the *window* the first has it half-updated. Pre-splitting, a
  `malloc` hit did minimal free-list surgery (mark allocated, unlink, return) and the
  list was often near-empty → tiny window, threads almost never collided → "worked".
  Splitting made every hit do *more* unsynchronized work — remove the found block
  **and** insert a leftover, fixing several links — and left many small leftover nodes
  on the list. Bigger critical section + longer list = the window widened from "never"
  to "often". Nothing new broke; exposure went up.
- **The gdb tell (`-O0`).** In `traverse_free_list`, a free-list node's header read as
  a heap address at line 46 but as `65` (`64 | 1` = size-64 **allocated**) two lines
  later at the use site — with no code in between. A stack-confirmed local disagreeing
  with the memory it was just read from ⇒ another thread wrote it mid-traversal. Two
  giveaways of the race: (a) a node on the *free* list marked *allocated*, and (b)
  memory changing between two adjacent lines that run no code. `info threads` at the
  breakpoint shows >1 thread. (Note: the debug `// fprintf` reentrancy of bug #9 muddied
  this at first; removing all prints kept the crash, which is what pointed at threads.)
- **Decision:** documented, **not fixed** — this is a single-threaded allocator by
  design. Path to concurrency if needed: one global lock around
  `malloc`/`free`/`realloc`/`calloc` (simple, serializes all allocation), vs.
  per-arena / per-thread caches (tcmalloc/jemalloc style — scales, much more work).

---

## Nits / non-bugs
- `calloc.c:14` `total = nmemb * size;` re-does the multiply already done safely by `__builtin_mul_overflow` on line 11 — redundant, drop it.
- `HEADER_SIZE` → `BOUNDARY_TAG_SIZE` rename: clarity only, not a bug.
- `free` no longer calls `*header_address = CLEAR_BIT(...)` directly — confirm the alloc bit still gets cleared in the new free/merge flow.

---

## Lessons

**1. Pointer vs integer arithmetic — watch the type of the LEFT operand.**
- `(size_t *)ptr + 1` → scaled by `sizeof(size_t)` = **+8 bytes**
- `(size_t)*heap_start + 1` → plain integer = **+1 byte**

`ptr + 1` (typed pointer) and `*heap_start + 8` (integer address) reach the *same* slot. Bug #1 was `*heap_start + 1` (only +1 byte, 7 short). The trap is mixing both notations in the same list code. Rule: *pointer ± n is scaled; integer ± n is raw bytes.*

**2. Check the mechanism, not just the output.** A whole loop sat dead and unnoticed because only the result was ever verified. Build a heap-walker validator (`assert header == footer`, alignment, free nodes in-bounds, no cycle) and call it in debug builds at the top of `malloc`/`free`. It would have caught bugs 1, 2, 5, and 8 *at the source*.

**3. Reach for the tools sooner.**
- `coredumpctl gdb` — open the latest core immediately, don't guess.
- `strace -c -e trace=brk,mmap` — non-reuse of free blocks shows up here as a huge **brk** count → **sys** time (saw 229k vs 3k). Not user time.
- `ltrace` — which library calls (malloc/calloc/free) actually fire.
- `/usr/bin/time -v` — **user** explodes → fix your algorithm; **sys** explodes → cut syscalls.

**4. One invariant check beats a test per function.** Per-function unit tests are overkill here; *one* validator on the central data structure (the heap / free list) is where the payoff is, because that's where a local bug becomes global corruption.

**5. "It runs" ≠ "it's correct"; corruption is silent until it isn't.** The missing `calloc` (#3) passed every small test for weeks, then detonated on `ls / -ltrR`. For an LD_PRELOAD allocator: `nm -D` after every build — verify what you *export*, not just what you *output*.

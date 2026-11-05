# Next Steps

 is_block_valid fixes the nm symptom but the root cause is unestablished; the add_leftover_block merge is ruled out (A/B, 20/20 still crashed); core dumps not yet read.
 
# Untested — what the compatibility suite cannot tell you (2026-07-28)

As of this date the allocator passes 98/98 commands in COMPATIBILITY.md on exit
code, stdout AND stderr, with the threaded entries repeated (5 git commands x 50
runs, 5 parallel `-j8` builds, all byte-identical) and `make tests_run` at 21/21.

That is weaker evidence than it looks. **The suite proves that these 98 specific
invocations behave like glibc — nothing more.** `nm` sat in the list broken for two
rounds precisely because a listed program happened to exercise the bug. A bug that
no listed program triggers produces output indistinguishable from correctness.

The three areas below are not covered by any command in the list, and are not
covered by the unit tests either.

## A. `fork()` while another thread holds the heap mutex

The single most likely remaining defect, because the suite structurally cannot
reach it: every listed command is one process doing its own allocation.

`heap_start_mutex` is a plain `PTHREAD_MUTEX_INITIALIZER`. `fork()` copies only the
calling thread, but it copies the *whole address space* — including the mutex in
whatever state it was in. If any other thread was inside `malloc`/`free`/`realloc`
at the moment of the fork, the child inherits a mutex that is locked by a thread
that does not exist in the child. The child's first allocation blocks forever.

Reproduce: spawn N threads churning allocations, fork repeatedly from the main
thread, and have the child do a single `malloc` then `_exit(0)`. Anything other
than prompt exit is the bug. Use `alarm()` in the child so a hang shows up as a
signal rather than a hung test run.

Fix, if confirmed: `pthread_atfork()` to take the lock before the fork and release
it in both parent and child. Note glibc's own allocator does exactly this.

Why it matters in practice: every shell pipeline forks. The reason it has not been
seen is that the listed programs fork *before* they allocate much, or are
single-threaded.

## B. Allocation failure under `setrlimit(RLIMIT_DATA, ...)`

Round 2 found a real bug here — `add_free_block` overwrote the epilogue before it
discovered `allocate_block` had failed, so a failed `malloc` returned NULL but left
`check_heap()` at -1. It was fixed, and the fix was verified then.

`src/malloc.c` and `src/realloc.c` have both been restructured since (the
`create_free_block` / `get_suitable_block` split, the `block_size == 0` guards, the
`check_available_space` rewrite). **That verification has not been repeated against
the current code.**

Reproduce: `setrlimit(RLIMIT_DATA, 8192)` before a `malloc(1 << 20)`. Assert three
things, not one — the call returns NULL, `check_heap()` is unchanged, and a normal
allocation afterwards still succeeds. The last one is what catches a half-written
block or a lock left held.

Do the same for `realloc` and `calloc`, which take different paths to `sbrk`.

## C. No sanitizer or valgrind run, ever

Every result so far comes from `check_heap` / `check_free_list` — checkers written
against the same mental model as the allocator. They cannot see a bug that model
does not anticipate, and they only inspect blocks reachable by walking the heap.

Neither ASan nor valgrind can instrument this allocator directly (they replace
malloc, which is the thing under test). Two things that do work:

- Build the churn harness against the library and run it under
  `valgrind --tool=memcheck --soname-synonyms=somalloc=NONE`, which stops valgrind
  from intercepting and lets it see the raw `sbrk` region.
- Compile the *library* with `-fsanitize=undefined`. UBSan does not replace malloc,
  so it works normally, and this code does a great deal of pointer arithmetic and
  integer arithmetic on `size_t` — exactly what UBSan is for.

## D. Two unit tests that cannot fail

Both pass against deliberately broken builds — verified by mutation:

- `Test(free, invalid_free)` uses `0x01234`, which is rejected by the old *and* new
  guard (not 16-aligned, far below the heap). Reverting `free.c` to
  `is_address_valid` leaves the suite at 21/21. A discriminating input has to be
  16-aligned, inside the heap, with a fake header whose bit 0 is set.
- `Test(malloc, size_max)` and `Test(realloc, size_max)` assert the return value but
  never call the allocator again, so a mutex left locked is invisible. Reinstating
  the missing `pthread_mutex_unlock` bug leaves the suite at 21/21. Note that an
  assertion alone turns the bug into a *hang*, not a failure — Criterion's
  `.timeout` attribute is what makes it report properly.

The general lesson, and the reason to write these down: **a test that has never
been seen to fail has not been shown to work.** Break the code on purpose and
confirm the test catches it. Every fix in this project should get that treatment.

---

## Debugging reflexes learned (see DEBUGGING_CHEATSHEET.md)

- Correct output != correct implementation. Verify the feature RUNS (count it).
- `real` = how slow; `user` vs `sys` = where (user=your code, sys=syscalls).
- Stuck after an edit? Get a backtrace, don't re-read and guess.
- "I fixed it" = "I ran the check and saw it pass."
- A special case that "works" often only works because another bug hides that path.

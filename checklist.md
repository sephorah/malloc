realloc — one test per branch, since each is a distinct code path:
- ptr == NULL → behaves like malloc
- size == 0 → frees, returns NULL
- shrink in place → returns the same address, and check_heap/check_free_list still pass (this is the one that catches your old bug — assert the address is unchanged and the leftover became a free block)
- grow → new address, and the old contents are preserved (strcpy before, str_eq after)

calloc:
- returned memory is actually zeroed (write nothing, assert all bytes 0)
- nmemb * size overflow returns NULL (if you handle it — if you don't, that's worth knowing)




SHIP CHECKLIST — 3h if the segfault survives, otherwise ~5h 
Cut rule: if out of time, cut upward from step 6. Steps 1-5 are non-negotiable.

[x] 1. VALIDATOR HUNT ON THE SEGFAULT — 1h HARD TIMEBOX
       wire check_heap() + check_valid_list() at the end of malloc and free
       (debug build), run `LD_PRELOAD=./libmalloc.so git status`.
       First failing call names the corrupting operation.
       → names it: fix it (+1-2h). Best outcome, I arrive with a working allocator.
       → 60 min, no lead: STOP. go to step 2. don't negotiate here.

[x] 2. BACKTRACE — 15 min   (only if step 1 didn't produce a fix)
       make debug
       LD_PRELOAD=./libmalloc.so git status
       coredumpctl dump git --output=/tmp/git.core
       gdb -q -nx -ex "set debuginfod enabled off" -ex bt $(which git) /tmp/git.core
       save the backtrace → goes in README + email

[x] 3. STRIP DEBUG PRINTFS — 5 min
       the Floyd's-walk prints ("start ... slow ... fast ...", "next fast null").
       `make tests_run` must output the 9/9 synthesis and nothing else.
       (stray printf inside a malloc override can recurse into my own allocator)

[x] 4. VERIFY THE FOUR COMMANDS — 10 min
       cat / python3 -c pass / ls -la already exit 0.
       `ls /usr -ltrR` exits 1 — compare against a run WITHOUT LD_PRELOAD.
       both 1 = permission-denied noise, say so. only mine = second bug, know it first.

[x] 5. .GITIGNORE + ONE CLEAN COMMIT — 15 min
       libmalloc.so, *.gcda, *.gcno, unit_tests, temp/, .vscode/, example*, chatgpt_advice
       tree must be clean when he pulls

[ ] 6. README — 45 min, four sections, no more
       [ ] build / run / test commands
       [ ] the validator: which invariants, why one invariant checker beats
           per-function tests, which 4 bugs it would have caught at the source
       [ ] silent bugs: missing calloc passed small tests for weeks then detonated
           on ls -ltrR; the memset(nmemb) tail-garbage one.
           Point: correct output != correct implementation
       [ ] known limits: git status segfault + my diagnosis, validator negative
           paths untested, single-threaded/no locks, first-fit fragments,
           perf table (line ~164) leading with the ~1.25x user-time figure,
           note that single runs are noisy

[ ] 7. EMAIL — 20 min
       one sentence on the delay, no justification. then: what it does, what the
       validator is, what's broken and where I think it is, what I'd do next. repo link. send.

DELIBERATELY CUT (do NOT do these before sending):
  - the 8 validator negative tests   → 4h to prove synthetic corruption fails;
                                        1h pointing the validator at the REAL
                                        corruption gets the fix AND the evidence
  - calloc / realloc / coalescing test batteries → test count is the least
                                        differentiated signal; 9 passing already
                                        establishes that I test
  - get_next_element + helper-function refactors → zero signal, and refactoring
                                        around a live segfault reads as motion
                                        where judgment was needed
  - filling in the BUGS.md "why" answers → that's for me, not for him
  - re-running the benchmark → numbers at line ~164 are good enough, use as-is

KEEP: the 9 passing tests (written, passing, free to ship) and both validators
      (check_heap / check_valid_list) — the validators are the best thing in this
      project and the whole README story. Lesson isn't "test-first", it's
      "invariant-first": tests check what I thought to check, the validator checks
      the structure on every op whether I thought of it or not.

--------------------------------------------------------------------------

SETUP (~20 min)
[x] A1  fix inverted return in check_boundary_tag
[x] A2  fix zero-advance loop in check_heap (size_tmp by value)
[x] A3  fix bare `return;` in int check_heap
[x] A4  add Floyd's cycle detection to check_valid_list
[ ] helper: get header / footer from a payload pointer (malloc.c, traverse_free_list, )
[ ] helpers functions to get prev and next blocks (merge free blocks)


AFTERNOON 1 — VALIDATOR TESTS (~4h)
[ ] positive: healthy alloc/free → check_heap()==SUCCESS && check_valid_list()==SUCCESS
[ ] neg: header ≠ footer
[ ] neg: invalid size (0)
[ ] neg: invalid size (unaligned)
[ ] neg: invalid footer address (inject via corrupt size)
[ ] neg: invalid free-list node address
[ ] neg: allocated block in free list
[ ] neg: cyclic free list → ERROR, not hang    (.timeout = 1)


AFTERNOON 2 — ALLOCATOR TESTS (~5h)

malloc / free
[x] simple malloc / free
[x] malloc reuses a freed block
[x] malloc size 0
[x] large allocation (> page size, multiple sbrk)
[x] returned pointer is 16-aligned
[x] free(NULL)
[x] double free
[x] churn: 20 mallocs, free every other, 20 more → check_valid_list()==SUCCESS

coalescing  (merge_free_blocks — 4 paths, most bug-prone code)
[ ] merge with next   (free A then B, adjacent)
[ ] merge with prev   (free B then A, adjacent)
[ ] merge both sides  (malloc A,B,C → free A, free C, free B)
[ ] no merge          (allocated neighbours both sides)
    → each: check_heap==SUCCESS, check_valid_list==SUCCESS,
      merged size == sum of parts

data integrity
[ ] 3 blocks, distinct byte pattern each, memset FULL payload,
    verify all patterns intact   (catches payload overrun into neighbour)
[ ] check whether epilogue and prologue are allocated

calloc
[ ] simple calloc
[ ] calloc zeroes the FULL region (check every byte)
[ ] calloc nmemb == 0
[ ] calloc size == 0
[ ] calloc integer overflow → NULL

realloc
[ ] simple realloc
[ ] realloc(NULL, n)
[ ] realloc(p, 0)
[ ] realloc larger
[ ] realloc smaller
[ ] realloc same size
[ ] realloc preserves contents (compare bytes before/after)

[ ] fix whatever these uncover — including the `git status` segfault (exit 139).
    The coalescing + churn tests are the most likely to reproduce it.
[ ] regression: pointer-scaling repro    ← cut first if behind

AFTERNOON 3 — SHIP (~4h)
[ ] .gitignore: libmalloc.so, temp/, .vscode/, chatgpt_advice, example*
[ ] commit clean tree
[ ] benchmark: tight alloc/free loop vs system malloc (isolates malloc from I/O,
    unlike `ls /`)   ← shrink to one number if behind
[ ] README: silent-bugs story + perf numbers + single-threaded/fragmentation tradeoffs


TWO CODE QUESTIONS TO RESOLVE WHILE WRITING THE TESTS
[ ] realloc.c → allocate_new_block() returns `ptr` when malloc fails.
    What does the caller then believe about the block's size? Compare to real realloc.
[ ] calloc.c → returns NULL for nmemb==0 || size==0.
    Check glibc's behaviour; match it or document the deviation.


[ ] replace skip to next element (get_next_element) wherever i can

BEFORE SENDING
[ ] Imagine you're the employee, you want to ask questions to check whether i understood what i wrote (e.g. why using size_t *)
[ ] builds clean under -Wall -Werror
[ ] every positive test passes
[ ] every negative test actually fails (exit 84)
[ ] `git status`, `ls / -ltrR`, `cat`, `python -c pass` all run clean under LD_PRELOAD
[ ] then email him



/* 

claude --resume ac8b8b94-7f9c-4eb9-b483-34dcaa712053

Write tests
then add detection cycle
then write README

Positive (do first):
- healthy alloc/free → check_heap()==SUCCESS && check_valid_list()==SUCCESS

Validator negatives:
- header ≠ footer
- invalid size (0 / unaligned)
- invalid footer address (inject via corrupt size)
- invalid free-list node address
- allocated block in free list
- cyclic free list → expect ERROR, not a hang ← this is the test that proves Floyd's works (and it would hang forever without fix A4)

Semantic (the class ls missed):
- calloc zeroes the full region + overflow returns NULL
- realloc preserves bytes (grow / shrink / same), realloc(NULL,n), realloc(p,0)
- malloc(0), free(NULL), returned pointer 16-aligned

Regression:
- the pointer-scaling minimal repro (the one bug that recurred 4×)

The dependency to keep in mind: A4 (Floyd's)
 and the cycle test are a pair — write the fix first, or the 
 test you wrote to verify it will be the thing that hangs. 
 Same with A1/A2: the positive test can't pass until the 
 inverted return and the zero-advance loop are fixed.

 https://chatgpt.com/c/6a3f05f5-f3bc-83ea-91bb-dabde2cd4548?mweb_fallback=1

Performance / benchmarking — the biggest gap

- Benchmark alloc/free throughput vs system malloc (latency is the product at CFM)
- Measure fragmentation
- Be ready to discuss first-fit vs best-fit / segregated-list tradeoffs — you fragment badly with first-fit, so know it and own it

Thread safety — discuss, don't necessarily implement

- State clearly: "single-threaded, no locks"
- Explain the path to concurrency: global lock vs per-arena (tcmalloc/jemalloc style) and the tradeoff

README story — the judgment, not the code

- "It passed ls -ltrR / and still had silent bugs (calloc not zeroing, block-reuse loop never ran)"
- "So I test by risk + invariants, not by volume"
- Include the perf numbers + the thread-safety/fragmentation tradeoffs you chose

The framing to keep in mind

- The artifact is table stakes — a correct malloc isn't impressive by itself
- The signal is the story: came back to an embarrassing project, found your own silent bugs, built a methodology
- Don't optimize for one person's opinion — the methodology you keep is the real win; his respect is the side effect
*/



----------------------
// ENLEVER COMMENTAIRES, FAIRE FICHIERS POUR FONCTIONS (5 max par fichier)
// LD_PRELOAD=./libmalloc.so ls / -ltrR
// search test cases for memory allocator
// rendre fonctions statiques
// store the address of the first free block in prologue
// next point to payload
// gdb --args ls / -ltrR
// set environment LD_PRELOAD ./libmalloc.so

┌────────┬─────────────┬───────────────┬───────┐
│ Metric │ Your malloc │ System malloc │ Ratio │
├────────┼─────────────┼───────────────┼───────┤
│ real   │ 50.50s      │ 44.82s        │ 1.13× │
├────────┼─────────────┼───────────────┼───────┤
│ user   │ 7.74s       │ 6.19s         │ 1.25× │
├────────┼─────────────┼───────────────┼───────┤
│ sys    │ 20.57s      │ 18.42s        │ 1.12× │
└────────┴─────────────┴───────────────┴───────┘

Use tools like gdb, check time (user and sys) compared to reference 
output != working correctly

Before coding, write down what must always be true (invariants) and what could be silently wrong (alignment, zeroing). Then make sure each distinct code path is actually exercised by a test — happy path and the rare ones (reuse, coalesce-both-sides, overflow). Not every function; the risky paths and the silent failures.

claude --resume ac8b8b94-7f9c-4eb9-b483-34dcaa712053

Four pass, but git status segfaults under your allocator (exit 139). That's a genuine new bug â git does something ls/cat/python don't (likely a realloc pattern, a very large allocation, or a specific free/coalesce sequence). Your suite just earned its keep: it caught a real failure that ls -lR never exposed.

------------------------------------------------------

Single runs are noisy — the 1.13× real ratio could swing either way on a rerun since so much depends on filesystem cache state. The user-time signal (~1.25×) is the stable one.

If you want a cleaner allocator benchmark that isolates malloc from filesystem I/O, a tight alloc/free loop is far more revealing than ls /. Want me to write one? It'd let you see per-size-class behavior (small vs. large blocks, reuse from the free list, coalescing cost) that ls completely hides.

Where's the ~25% user-time gap likely coming from in your design? Your first-fit walk of the explicit free list is the prime suspect — glibc uses size-segregated bins for near-O(1) lookup, while you do a linear scan that grows with free-list length. Worth profiling if you want to close the gap.

-------------------------------------------------------

IDEA WORTH KEEPING (only if time allows):
Fixed-seed randomized test — malloc/free/realloc in random order, payload byte
patterns verified after each op, checker called after every op. Covers exact-fit /
no-split / head-middle-tail removal by construction.

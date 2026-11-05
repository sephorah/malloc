/*
 * churn_rand.c — fixed-seed randomized allocator fuzzer / reproducer.
 *
 * Single-threaded on purpose: it exercises the split + coalesce paths with a
 * varied, random sequence of sizes and free/alloc order that the fixed-pattern
 * unit tests never hit. This is the reproducer that surfaced the split/coalesce
 * corruption which `ls`, simple churn, and single-threaded `git status` all
 * missed — a rich enough SEQUENCE, not concurrency, is what triggers it.
 *
 * Why fixed-seed: the run is deterministic per seed, so a crash reproduces
 * identically every time — you can set a gdb watchpoint on the corrupted address
 * and re-run into the exact same state. (ASLR off under gdb makes it repeatable.)
 *
 * Two independent detectors:
 *   1. Per-slot fill: each block is filled with a known byte and verified intact
 *      on the next touch — catches SILENT payload corruption even without a crash.
 *   2. Validators: check_heap() + check_free_list() are called after EVERY malloc
 *      and free (resolved from the preloaded lib). Either returning < 0 means the
 *      heap / free list is corrupt; we abort immediately, so the FIRST failing op
 *      is named instead of crashing later at the victim site.
 *      (check_free_list() < 0 fires on an allocated block left on the free list —
 *      exactly the split/coalesce class we're hunting.)
 *
 * Build + run (LD_PRELOAD supplies malloc/free AND the validators):
 *     make
 *     cc tests/churn_rand.c -ldl -o churn_rand
 *     LD_PRELOAD=./libmalloc.so ./churn_rand 1     # seed 1 (default if omitted)
 *     for s in $(seq 1 8); do LD_PRELOAD=./libmalloc.so ./churn_rand $s; done
 *
 * Expected once the allocator is correct: "survived seed N" (exit 0), every seed.
 *   SIGSEGV (139)                 -> allocator crashed at the victim site
 *   "check_* FAILED after <op>"   -> validator caught the culprit op (exit 3)
 *   "OVERWRITE ..."               -> a neighbour overwrote a live payload (exit 2)
 *
 * Without LD_PRELOAD the validators are absent (dlsym -> NULL) and are skipped;
 * the run then just checks glibc + the fill logic, and must print "survived".
 */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLOTS 128
#define ITERS 300000
#define MAX_PAYLOAD 512

static int (*g_check_heap)(void);
static int (*g_check_free_list)(void);

/* Call the validators after an op; abort at the first sign of corruption. */
static void validate(const char *op, unsigned seed, int iter, int slot)
{
    if (g_check_heap && g_check_heap() < 0) {
        // fprintf(stderr, "check_heap FAILED after %s: seed %u iter %d slot %d\n",
                op, seed, iter, slot);
                abort();
    }
    if (g_check_free_list && g_check_free_list() < 0) {
        // fprintf(stderr, "check_free_list FAILED after %s: seed %u iter %d slot %d\n",
                op, seed, iter, slot);
                abort();
    }
}

int main(int argc, char **argv)
{
    unsigned seed = (argc > 1) ? (unsigned)atoi(argv[1]) : 1;
    void *slots[SLOTS] = {0};
    size_t sizes[SLOTS] = {0};
    unsigned char fill[SLOTS] = {0};

    /* Resolve validators from the preloaded lib; NULL (skipped) without preload. */
    g_check_heap = dlsym(RTLD_DEFAULT, "check_heap");
    g_check_free_list = dlsym(RTLD_DEFAULT, "check_free_list");

    srand(seed);
    for (int i = 0; i < ITERS; i++) {
        int s = rand() % SLOTS;

        /* if this slot is live, verify its fill is intact, then free it */
        if (slots[s]) {
            unsigned char *p = slots[s];
            for (size_t k = 0; k < sizes[s]; k++) {
                if (p[k] != fill[s]) {
                    // fprintf(stderr,
                        "OVERWRITE seed %u iter %d slot %d off %zu: got %02x want %02x\n",
                        seed, i, s, k, p[k], fill[s]);
                        return 2;
                }
            }
            free(slots[s]);
            slots[s] = NULL;
            validate("free", seed, i, s);
        }

        size_t sz = 8 + (size_t)(rand() % MAX_PAYLOAD);
        slots[s] = malloc(sz);
        if (slots[s]) {
            fill[s] = (unsigned char)(s * 7 + i); /* distinct-ish per (slot,time) */
            sizes[s] = sz;
            memset(slots[s], fill[s], sz);
        }
        validate("malloc", seed, i, s);
    }

    for (int s = 0; s < SLOTS; s++)
        if (slots[s])
            free(slots[s]);

    printf("survived seed %u\n", seed);
    return 0;
}

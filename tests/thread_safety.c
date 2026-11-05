/*
 * thread_safety.c — reproducer for the concurrency race, NOT a correctness test.
 *
 * This allocator is single-threaded: `heap_start` and every free-list link is
 * mutated with no lock. Under LD_PRELOAD into a multithreaded program (ugrep,
 * git status with core.preloadindex, make -j, ...) several threads call
 * malloc/free at once, interleave their edits to the shared free list, and
 * corrupt it -> SIGSEGV. It is intermittent by nature: most interleavings
 * happen not to collide, which is why the same command "works" on a rerun.
 *
 * This program removes the luck. It hammers the allocator from many threads so
 * the collision happens on essentially every run.
 *
 * Build + run (segfaults reliably; that IS the reproduction):
 *     make                 # build libmalloc.so
 *     make thread_test     # compile this and run it under LD_PRELOAD
 * or manually:
 *     cc tests/thread_safety.c -pthread -o thread_test
 *     LD_PRELOAD=./libmalloc.so ./thread_test
 *
 * Expected today: "Segmentation fault (core dumped)" (exit 139).
 * "survived" printed = the race did not trigger this run; re-run a few times.
 *
 * NOTE: run it WITHOUT LD_PRELOAD and it prints "survived" every time — that
 * confirms the test itself is race-free; the corruption is in the allocator.
 */
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define THREADS 8
#define ITERS   20000
#define SLOTS   16

static void *worker(void *arg)
{
    (void)arg;
    void *slots[SLOTS] = {0};

    for (int i = 0; i < ITERS; i++) {
        int s = i % SLOTS;
        if (slots[s]) {
            free(slots[s]);
            slots[s] = NULL;
        }
        size_t sz = 8 + (i * 37) % 512;   /* varied sizes -> exercise coalescing paths */
        slots[s] = malloc(sz);
        if (slots[s])
            memset(slots[s], 0xAB, sz);
    }
    for (int s = 0; s < SLOTS; s++)
        if (slots[s])
            free(slots[s]);
    return NULL;
}

int main(void)
{
    pthread_t t[THREADS];

    for (int i = 0; i < THREADS; i++)
        pthread_create(&t[i], NULL, worker, NULL);
    for (int i = 0; i < THREADS; i++)
        pthread_join(t[i], NULL);

    printf("survived\n");
    return 0;
}

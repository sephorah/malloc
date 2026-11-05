# Debugging Cheatsheet (malloc / LD_PRELOAD)

**Theme: gather evidence, don't guess.** Run, don't re-read.

```bash
LD_PRELOAD=./libmalloc.so <cmd>     # force the program to use OUR malloc
```

## Quick triage

| Symptom | First command |
|---------|---------------|
| Wrong output | `diff` with/without preload (§correctness) |
| Crash (segfault/abort) | core dump + `gdb bt` (§gdb) |
| Foreign program asserts a *value* | calloc not zeroing → check `memset` length |
| Hang | `gdb -p <pid>` then `bt` |
| Intermittent crash (passes on rerun) | `info threads` — is the caller multithreaded? then force it single-threaded to confirm a race |
| Slow | `/usr/bin/time -v` then `strace -c` |
| "It allocates wrong" | `nm -D` — am I even exporting that symbol? |

## §exports — what do I actually override? (run after EVERY build)
```bash
nm -D libmalloc.so | grep -iE 'malloc|calloc|realloc|free'   # T = defined, U = missing
```
Every entry point a program can call MUST show `T`. A missing one (e.g. calloc) escapes to glibc → both allocators corrupt the shared heap.

## §correctness
```bash
LD_PRELOAD=./libmalloc.so <cmd> >ours 2>/dev/null; echo $?
<cmd>                             >base 2>/dev/null; echo $?
diff ours base && echo IDENTICAL          # compare output AND exit codes
```

## §gdb
```bash
make debug                                 # build with -g3 -O0 for names + line numbers
LD_PRELOAD=./libmalloc.so ./prog           # let it crash → systemd grabs the core
coredumpctl gdb                            # open NEWEST core (or: coredumpctl gdb ./prog)
gdb -p <pid>                               # attach to a HANGING process
```
Inside gdb:
| Command | Does |
|---------|------|
| `bt` | backtrace (call stack at crash) |
| `frame N` / `f N` | jump to stack frame N |
| `info locals` / `info args` | variables in current frame |
| `p expr` / `p/x expr` | print value (hex) |
| `p/x *header` | the VALUE at header — real block = small ×16; `0x7ffff7…` = foreign |
| `call func(args)` | RUN a function in the live process (`call check_heap()`) |
| `p check_valid_list()` | same thing, but prints the return value — use this one |
| `info proc mappings` | memory map: which range is heap / stack / libc / libgcc |
| `info proc` | pid, exe, cwd of the process being debugged |
| `info symbol 0xADDR` | name the symbol covering an address |
| `break file.c:91` / `b func` | set breakpoint |
| `break func if x==0` | conditional breakpoint |
| `run` / `continue` / `c` | start / resume |
| `next`/`n`, `step`/`s`, `finish` | over / into / out of |
| `watch *p` | stop when memory at p changes |
| `info threads` | list all threads; **>1 = concurrent callers** → suspect a data race on shared state (`heap_start`, free-list links) |
| `thread N` | switch to thread N, then `bt` it |

`coredumpctl list` to see recent cores. Address vs value: `header` = where it lives; `*header` = what it holds.

### Is this address mine?
```
info symbol 0x00007ffff7fbcf6e     # names the library+symbol if it's in one
info proc mappings                 # which mapping the address falls in
```
`0x7ffff7…` = shared-library / dynamic-loader region (gdb disables ASLR, so ld.so and
libc land there predictably). The `sbrk` heap is far BELOW that, just past the program
break. A `0x7ffff7…` value in a header, footer, or free-list next/prev slot is a
**foreign pointer** — not my metadata. Same shape as BUGS.md #3.

### Preload the lib without retyping it every session
```bash
gdb -ex 'set exec-wrapper env LD_PRELOAD=./libmalloc.so' --args git status
```
`exec-wrapper` injects the preload into the INFERIOR only, so gdb itself keeps the system
malloc. Do NOT `LD_PRELOAD=./libmalloc.so gdb …` — that preloads into gdb's own process, so
gdb allocates through the buggy code and can crash/hang on your bug, not the program's.
Make it permanent: put that one `set exec-wrapper` line in a project `./.gdbinit` (needs
`set auto-load safe-path .` in `~/.gdbinit` once); then plain `gdb --args git status` preloads.

### Run the validators under gdb (no printf needed)
```
gdb --args git status
set environment LD_PRELOAD ./libmalloc.so     # or the exec-wrapper form above
p check_heap()                     # call on demand, anywhere
break free
condition 1 check_valid_list() != SUCCESS
run                                # stops at the op AFTER the one that corrupted → bt
```
Slow (an inferior call per `free`) — fine for `git status`, painful for `ls -ltrR`.
If the inferior calls get flaky mid-allocation: add a `validator_failed()` function that
the validator calls on its error path, then plain `break validator_failed` — no inferior
calls, no conditions, and it fires exactly at the detection point.

## §strace — syscalls (brk, mmap, open…)
```bash
strace -f -c -e trace=brk,mmap <cmd>       # -c = summary table (count+time per syscall)
```
High `brk` count = allocator not reusing the free list (saw 229k broken vs 3k fixed).

## §ltrace — library calls (malloc, calloc, free…)
```bash
ltrace -f -e 'malloc+calloc+realloc+free' <cmd> 2>&1 | head
```
Proves WHICH allocator entry points a program uses, and the return values.

## §time — where the time goes
```bash
/usr/bin/time -v <cmd>
```
**real** = wall clock · **user** = your code (algorithm) · **sys** = kernel/syscalls.
Rule: user explodes → fix your code; sys explodes → cut syscalls.

## §valgrind — invalid reads/writes/leaks WITHOUT a crash
```bash
valgrind --leak-check=full LD_PRELOAD=./libmalloc.so ./prog
```

## A/B against last good version
```bash
git stash && make re   # baseline → measure → git stash pop && make re
```

## Minimal repro
Smallest standalone program (a few malloc/free), `gcc -g`, run under the lib. Fast crash, short backtrace.

---

## Reflexes
1. **`nm -D` after every build** — verify exports, not just output.
2. **Corruption ≠ crash.** A clean run proves nothing; the bug detonates later. Trust a crash or a validator only.
3. **The crash site is the victim, not the culprit.** Ask: was memory already poisoned when I arrived? Check `*header`'s value.
4. **One root cause, many masks.** abort↔segfault, crashes here not there = latent memory corruption, not separate bugs.
5. **Read the failure CLASS.** Value assertion in a foreign program → calloc didn't zero. Segfault in your code → bad pointer/header.
6. **Evidence before re-reading code.** A backtrace + `*header` + `nm -D` ends most guessing in minutes.
7. **Check your invariants** (header == footer; list back-pointers are valid addresses). A validator catches bugs at the source, not 200 calls later.
8. **"Fixed" = ran the check and saw it pass** — never "changed the line I suspected."
9. **Never `printf`/`// fprintf` from inside malloc/free.** `stdout` is buffered, so the
   first `printf` mallocs its buffer → re-enters my allocator (and buffered output is
   lost if the process dies). `stderr` being unbuffered *reduces* the risk but does
   **not** eliminate it: `// fprintf` can still call `malloc` internally (wide-char/locale,
   float, wide fields), re-enter the allocator, grab the very block I'm mid-traversal on,
   and leave my in-flight reads stale — a Heisenbug where the print corrupts the state it
   was meant to observe (BUGS.md #9). Only `write(2, …)` (raw syscall, no allocation) is
   safe — `snprintf` into a stack buffer, then `write`. Better still: `p check_heap()` in
   gdb, no output code at all. Tell it's biting you: a `-O0` local disagrees with the
   memory it was just read from, with no code in between.
10. **Intermittent segfault under LD_PRELOAD → suspect concurrency, not just my logic.**
    "Works on rerun / same command" is the *signature of a data race*, not a fixed bug.
    Run `info threads` at the crash; if >1, the caller is multithreaded and my unlocked
    allocator is racing on `heap_start` / the free list. Confirm by forcing the program
    single-threaded (e.g. `git -c core.preloadindex=false status`) — clean single-threaded
    = a race, not a logic bug. Trap to avoid: a change that *correlates* with a new crash
    (e.g. I just added splitting) is the prime suspect, but a change can also *widen the
    window* on a pre-existing latent race rather than *be* the bug. Test it, don't assume it.

# Compatibility test log

gdb -ex 'set exec-wrapper env LD_PRELOAD=./libmalloc.so' --args ls / -ltRr

Every command below was run twice — once normally, once with
`LD_PRELOAD=./libmalloc.so` — against a clean `make re` release build
(`-fPIC -Werror -Wall -Wextra -fvisibility=hidden`, no `-g`).

Host: Fedora 39, glibc 2.38, Linux 6.11.9, Intel i5-1135G7 (4 cores / 8 threads, 1 NUMA node).

**Two comparison levels are used, and they are not equivalent:**

| Level | What was checked |
|---|---|
| **full** | exit code **and** byte-identical stdout **and** stderr vs. the unpreloaded run |
| **crash-only** | exit code only — catches signals, would not catch silently wrong output |

Sections 1–5 are *full*. Section 6 is *crash-only* and is weaker evidence.

> **Round 2 (2026-07-28):** everything from here to the end of section 6 describes
> the allocator **before** the malloc/free/realloc/calloc fixes. `curl` has since
> been fixed; `nm` has not. See [Round 2 — re-test after allocator fixes](#round-2--re-test-after-allocator-fixes-2026-07-28)
> at the end of this file for what was re-run and what the results were.
>
> **Round 3 (2026-08-05):** `nm` no longer crashes either. See
> [Round 3](#round-3--re-test-after-the-free-is_block_valid-guard-2026-08-05) at the
> very end of this file. The `nm` and `curl` sections immediately below are kept as
> the historical record of rounds 1–2 and no longer describe current behaviour.

---

## Results at a glance

| | count |
|---|---|
| Commands run | 87 |
| Behaved identically to glibc | 85 |
| **Failed** | **2** (`nm`, `curl`) |
| False positives ruled out | 3 (`ldd`, `git` rc=128, `c++filt`) |

---

## FAILURES

### `nm` — segfault on every object file

Deterministic. Reproduces on a 22 KB file as readily as on libc.

```
$ echo 'int main(void){return 0;}' > tiny.c && gcc tiny.c -o tiny
$ LD_PRELOAD=./libmalloc.so nm tiny
Segmentation fault (core dumped)          rc=139
```

Every one of these crashes with rc=139:

| command | result |
|---|---|
| `nm tiny` (5-line program) | SEGV |
| `nm e.o` (empty object file) | SEGV |
| `nm /bin/ls` | SEGV |
| `nm -D /bin/ls` | SEGV |
| `nm -D /usr/lib64/libc.so.6` | SEGV |
| `nm -D /usr/lib64/libstdc++.so.6` | SEGV |
| `nm -C /usr/lib64/libstdc++.so.6` | SEGV |
| `nm -D /usr/lib64/libm.so.6` | SEGV |
| `nm -D /usr/lib64/libcrypto.so` | SEGV |
| `nm -D ./libmalloc.so` | SEGV |
| `nm --version` | **ok** — never opens an object file |

**Output is silently truncated**, which is worse than the crash itself:

```
$ nm -D /usr/lib64/libc.so.6 | wc -l                      → 3078
$ LD_PRELOAD=./libmalloc.so nm -D /usr/lib64/libc.so.6 | wc -l  → 2989
```

89 lines lost. The crash happens during `exit`, so the stdout buffer is never
flushed — a caller redirecting to a file gets a truncated result.

### `curl --version` — segfault

```
$ LD_PRELOAD=./libmalloc.so curl --version
Segmentation fault (core dumped)          rc=139
```

Not a packaging artifact — both builds present on this machine crash:

| binary | glibc | ours |
|---|---|---|
| `/usr/bin/curl` (Fedora) | rc=0 | **rc=139** |
| `/home/linuxbrew/.linuxbrew/bin/curl` | rc=0 | **rc=139** |

Other curl invocations are fine: `curl --help` (rc=0), `curl -s file:///etc/hostname` (rc=0).

### Shared characteristic

Both crashes occur **at process exit**, inside `free`'s coalescing path, invoked
from a C++ static destructor running under `__run_exit_handlers`. Neither program
links the responsible library directly — it is pulled in at runtime, which is why
two apparently unrelated tools fail the same way.

Backtrace command:

```
gdb -q -batch -ex run -ex bt --args env LD_PRELOAD=./libmalloc.so nm tiny
```

---

## 1. coreutils — full comparison

| command | cwd | result |
|---|---|---|
| `ls -la /usr/bin` | `/tmp` | identical (253,772 B) |
| `ls -ltRr /usr/include` | `/tmp` | identical (1,423,876 B) |
| `cat /etc/services` | `/tmp` | identical (4,953,598 B) |
| `sort /usr/share/hwdata/pci.ids \| head -100` | `/tmp` | identical |
| `find . -name '*.h' \| xargs wc -l \| tail -1` | `/usr/include` | identical |
| `du -sh .` | `/usr/include` | identical |
| `stat * \| md5sum` | `/usr/bin` | identical |
| `cat /etc/services \| tr -s ' ' \| sort \| uniq -c \| tail -20` | `/tmp` | identical |

## 2. text processing — full comparison

| command | cwd | result |
|---|---|---|
| `grep -rn 'typedef struct' . \| wc -l` | `/usr/include` | identical |
| `grep -Ec '[a-z]+' /usr/share/hwdata/pci.ids` | `/tmp` | identical |
| `sed 's/[aeiou]/X/g' /usr/share/hwdata/pci.ids \| md5sum` | `/tmp` | identical |
| `awk '{n+=NF} END{print n}' /usr/share/hwdata/pci.ids` | `/tmp` | identical |
| `find . -type f -name '*.so*' \| sort \| md5sum` | `/usr` | identical |

## 3. git — full comparison (multi-threaded)

All in the `malloc` repo unless noted.

| command | result |
|---|---|
| `git status --porcelain` | identical |
| `git status --porcelain` (nanotekspice) | identical |
| `git log --stat \| md5sum` | identical |
| `git diff \| md5sum` | identical |
| `git diff --cached \| md5sum` | identical |
| `git rev-list --all \| md5sum` | identical |
| `git ls-files \| md5sum` | identical |
| `git show HEAD \| md5sum` | identical |
| `git add -A --dry-run \| md5sum` | identical |
| `git rev-list --all \| git cat-file --batch \| md5sum` | identical |
| `git fsck --no-progress \| md5sum` | identical |
| `git blame README.md \| md5sum` | identical |
| `git archive HEAD \| wc -c` | identical |

`git push` is known not to work and was not tested.

## 4. compilers, build tools, binutils — full comparison

| command | result |
|---|---|
| `gcc -O2 -x c - -o /tmp/g1 && /tmp/g1` | identical |
| `g++ -c src/Simulator.cpp -iquote include/ -o /tmp/s1.o` | **byte-identical `.o`** |
| `g++ -O2 -c src/Simulator.cpp -iquote include/ -o /tmp/s2.o` | **byte-identical `.o`** |
| `gcc -E -x c++ src/Simulator.cpp -iquote include/ \| wc -l` | identical (95,189 lines) |
| `g++ -S src/AComponent.cpp -iquote include/ -o - \| md5sum` | identical |
| `objdump -d /bin/ls` | identical |
| `objdump -d /usr/lib64/libc.so.6` | identical |
| `objdump -T /usr/lib64/libc.so.6` | identical |
| `readelf -a /usr/lib64/libc.so.6` | identical |
| `readelf -s /usr/lib64/libc.so.6` | identical |
| `strings /usr/lib64/libc.so.6` | identical |
| `size /usr/lib64/libc.so.6` | identical |
| `ar t /usr/lib64/libc.a` | identical |
| `addr2line -e /bin/ls 0x1000` | identical |
| `c++filt` (standalone, and 3× repeated input) | identical |
| **`nm` (all forms)** | **SEGV — see FAILURES** |

### full builds

Run in `/home/sephorahaniambossou/delivery/old-tek-projects/nanotekspice`:

| command | result |
|---|---|
| `make re` | rc=0, **byte-identical binary** |
| `make fclean && make -j4` | rc=0, **byte-identical binary** |
| `make re` (this repo) | rc=0 |
| `make tests_run` (this repo) | 16/16 passing |

## 5. interpreters, archives, misc — full comparison

| command | result |
|---|---|
| `python3` — build a 200,000-entry dict, sum value lengths | identical |
| `python3 -c 'import json,re,os,sys; print(1)'` | identical |
| `python3 -m json.tool` | identical |
| `perl` — build a 200,000-key hash | identical |
| `perl -e 'print 1'` | identical |
| `bash` — 20,000-iteration arithmetic loop | identical |
| `node --version` | identical |
| `tar czf /tmp/a.tgz .` then `tar tzf \| sort \| md5sum` (`/usr/include`) | identical |
| `gzip -c pci.ids \| gunzip -c \| md5sum` | identical |
| `xz -c -0 pci.ids \| xz -dc \| md5sum` | identical |
| `openssl dgst -sha256 pci.ids` | identical |
| `openssl rand -hex 16` | identical |
| `openssl s_client -help` | identical |
| `diff pci.ids usb.ids \| md5sum` | identical |
| `md5sum *` in `/usr/bin` | identical |
| `find . -name '*.h' \| head -300 \| xargs md5sum \| md5sum` | identical |
| `sqlite3 :memory: 'create table…select sum(a)…'` | identical |
| `less -F pci.ids \| md5sum` | identical |
| `vim --version \| head -1` | identical |
| `ssh -V` | identical |
| **`curl --version`** | **SEGV — see FAILURES** |
| `curl --help` | identical |
| `curl -s file:///etc/hostname` | identical |

## 6. crash-only sweep

Exit code checked, output **not** compared. All returned rc < 128 except the
failures already listed.

`wget --version`, `gcc --version`, `make -v`, `cmake --version`,
`pkg-config --version`, `ranlib --version`, `sort /etc/services`,
`find /usr/include -name '*.h'`, `sed -n 1,5p /etc/services`,
`awk '{print}' /etc/services`, `grep -rn typedef /usr/include/stdio.h`,
`tar cf /dev/null /usr/include`, `ls -ltRr /usr/include`, `bash -lc 'echo hi'`.

All 14 binaries were confirmed present via `command -v` first, so none of these
was a `command not found` (rc=127) counted as a pass.

---

## False positives — not bugs

| apparent failure | actual cause |
|---|---|
| `ldd /bin/ls` output differs | `libmalloc.so` legitimately appears in the loaded-object list, and load addresses are ASLR-randomized. Differs run-to-run without the preload too. |
| `git status` / `git log` rc=128 | Sweep ran them outside a repository. `fatal: not a git repository` returns 128 **identically** with and without the preload. |
| `nm -D libstdc++ \| c++filt` output differs | Downstream of the `nm` crash. `c++filt` alone is clean. |

---

# Round 2 — re-test after allocator fixes (2026-07-28)

Re-run after six allocator fixes: `init_heap` moved inside the mutex, `sbrk`
failure checked in `add_free_block`, `realloc` returning `NULL` instead of the
old pointer on failure, `malloc_usable_size` subtracting both boundary tags,
`add_leftover_block` coalescing, and `calloc(0, n)` returning `malloc(0)`.

Build: working tree on top of `a19e595` (uncommitted), `make re`, same flags as
round 1 (`-fPIC -Werror -Wall -Wextra -fvisibility=hidden`, no `-g`). Same host.

## Harness — weaker than round 1, read this before trusting the table

```bash
o1=$(eval "$cmd" 2>e1);                 r1=$?   # glibc
o2=$(LD_PRELOAD=$LIB eval "$cmd" 2>e2); r2=$?   # ours
# compared: exit code, then stdout
```

Two differences from round 1's *full* level:

- **stderr was captured but never diffed.** This is exit code + stdout only.
- For the `| md5sum` commands, `$?` is the *last* stage of the pipeline, so a
  mid-pipe crash surfaces as a differing md5, not as an rc difference. Every md5
  matched, so the evidence holds — but it is not the same check as round 1.

## Results at a glance

| | count |
|---|---|
| Distinct commands re-run | 48 |
| Behaved identically to glibc | 47 |
| **Still failing** | **1** (`nm`) |
| **Fixed since round 1** | **1** (`curl`) |
| Round-1 commands *not* re-run | ~39 — see coverage gap below |

## The two round-1 failures

```
nm tiny                                    RC DIFF (glibc=0 ours=139)
nm /bin/ls                                 RC DIFF (glibc=0 ours=139)
nm -D /usr/lib64/libc.so.6                 RC DIFF (glibc=0 ours=139)
nm -D /usr/lib64/libstdc++.so.6            RC DIFF (glibc=0 ours=139)
nm -C /usr/lib64/libstdc++.so.6            RC DIFF (glibc=0 ours=139)
nm -D ./libmalloc.so                       RC DIFF (glibc=0 ours=139)
curl --version                             identical (rc=0)   <-- was SEGV
curl --help                                identical (rc=0)
```

`nm` truncation is unchanged: `nm -D /usr/lib64/libc.so.6 | wc -l` gives 3078
under glibc, 2989 under ours — the same 89 lines lost as round 1.

### `nm` — new diagnostic detail

Same signature as round 1: `free` → `merge_free_blocks` → `merge_prev_block`
(`src/merge_free_blocks.c:43`), from `llvm::PassRegistry::~PassRegistry()` under
`__run_exit_handlers`.

```bash
gdb -q -batch -ex 'set debuginfod enabled off' -ex run -ex 'bt 12' \
    --args env LD_PRELOAD=./libmalloc.so nm tiny
```

State at the fault:

```
header addr  = 0x5555555dc050        (inside our heap: 0x555555562010..0x5555556063b0)
header value = 0x736973796c61  ==  "alysis"      <-- ASCII, not a size
payload      = 0x00005555555fd830   0x0000555555562010
                    ^next               ^prev == heap_start exactly
```

Two facts that matter together: the header has been **overwritten with string
data** (it only survived the `is_allocated` check because `'a'` = 0x61 has bit 0
set), and the payload immediately after it holds a **well-formed free-list entry
at the head of the list**. So a live allocation's payload overlaps a free block's
header — the corruption is upstream of the crash, not in `merge_prev_block`.

Ruled out: the "aligned allocation is not interposed" theory. `libLLVM-17.so`
does import `_ZnwmSt11align_val_t` / `_ZdlPvSt11align_val_t`, and libstdc++
implements those via `posix_memalign@GLIBC_2.2.5`, which this library does not
interpose. But feeding our `free` a glibc chunk does **not** crash — it bails out
early because the glibc `prev_size` word reads as not-allocated:

```c
void *p = aligned_alloc(64, 128);   /* glibc's allocator */
free(p);                            /* ours — returns cleanly, leaks */
```

Also worth noting: `free()` never calls `is_block_valid` before trusting the
header, which is why this surfaces as a wild pointer at the footer write rather
than a clean rejection.

## 3. git — multi-threaded (13)

```
git status --porcelain
git log --stat | md5sum
git diff | md5sum
git rev-list --all | md5sum
git ls-files | md5sum
git show HEAD | md5sum
git add -A --dry-run | md5sum
git rev-list --all | git cat-file --batch | md5sum
git fsck --no-progress | md5sum
git blame README.md | md5sum
git archive HEAD | wc -c
git -c index.threads=8 status -uall --porcelain | md5sum     # new in round 2
git -c grep.threads=8 grep -n malloc -- src | md5sum         # new in round 2
```

All identical, rc=0. Additionally `git status --porcelain` ×60 in a loop:
60/60 identical, 0 crashes.

## 1 & 2. coreutils + text processing (8)

```
ls -la /usr/bin | md5sum
ls -ltRr /usr/include | md5sum
cat /etc/services | md5sum
sort /usr/share/hwdata/pci.ids | head -100 | md5sum
cat /etc/services | tr -s ' ' | sort | uniq -c | tail -20 | md5sum
grep -rn 'typedef struct' /usr/include | wc -l
sed 's/[aeiou]/X/g' /usr/share/hwdata/pci.ids | md5sum
awk '{n+=NF} END{print n}' /usr/share/hwdata/pci.ids
```

All identical.

## 4 & 5. binutils, interpreters, misc (13)

```
objdump -d /bin/ls | md5sum
objdump -T /usr/lib64/libc.so.6 | md5sum
readelf -a /usr/lib64/libc.so.6 | md5sum
strings /usr/lib64/libc.so.6 | md5sum
size /usr/lib64/libc.so.6
ar t /usr/lib64/libc.a | md5sum
c++filt _ZnwmSt11align_val_t
python3 -c 'd={i:str(i) for i in range(200000)}; print(sum(len(v) for v in d.values()))'
perl -e '%h=map{($_,$_)}(1..200000); print scalar keys %h'
tar czf /dev/null /usr/include && echo ok
openssl dgst -sha256 /usr/share/hwdata/pci.ids
sqlite3 :memory: 'create table t(a); insert into t values(1),(2); select sum(a) from t;'
gzip -c /usr/share/hwdata/pci.ids | gunzip -c | md5sum
```

All identical.

## Full builds

```bash
cd nanotekspice && make re                       # md5 4902be24f11dc47accd62196f54b3071
make fclean && LD_PRELOAD=$LIB make re           # md5 4902be24f11dc47accd62196f54b3071
```

**Byte-identical binary**, as in round 1. In this repo: `make re` clean under
`-Werror -Wall -Wextra`, `make tests_run` 16/16 passing.

## Crash-only repeats (×20 each)

```
ls -ltRr /usr/include
grep -r malloc /usr/include/malloc.h
find /usr/share/doc -maxdepth 2 -name README
sort -u /usr/share/dict/words
```

0 crashes each.

## Coverage gap — 48 of round 1's 87 commands

Round 2 sampled the list rather than re-running it. **Not** re-tested:

- **§1–2:** `find . -name '*.h' | xargs wc -l`, `du -sh .`, `stat * | md5sum`,
  `grep -Ec '[a-z]+'`, `find . -type f -name '*.so*'`
- **§3:** `git status` in nanotekspice, `git diff --cached`
- **§4:** the standalone `gcc`/`g++` invocations (`-O2 -x c -`, `-c Simulator.cpp`,
  `-E`, `-S`), `readelf -s`, `addr2line`, `c++filt` with 3× repeated input
- **§5:** `python3 -m json.tool`, the import test, `perl -e 'print 1'`, the bash
  20k loop, `node --version`, the `tar tzf` round-trip, `xz`, `openssl rand`,
  `openssl s_client -help`, `diff pci.ids usb.ids`, `md5sum *` in /usr/bin, the
  300-file `xargs md5sum`, `less -F`, `vim --version`, `ssh -V`,
  `curl -s file:///etc/hostname`
- **§6:** the entire 14-command crash-only sweep

## Discarded result

An early round-2 batch reported `failures=40/40` for four git commands. That was
a harness bug — the command string expanded to `git -c index.threads=8 git status …`,
double-prefixing `git`. Those results were discarded and the commands re-run
correctly; they appear as the two `-c …threads=8` entries in section 3 above,
both identical.

## Not compatibility findings, but found during round 2

### OOM left the heap inconsistent — since fixed

`add_free_block` overwrote the epilogue via `init_boundary_tag` *before* it
discovered `allocate_block` had failed, so a failed `malloc` returned `NULL`
correctly but damaged the heap. Reproduced with `setrlimit(RLIMIT_DATA, 8192)`
before a `malloc(1 << 20)`:

```
                              before fix   after fix
check_heap before OOM              3           3
malloc under OOM returned        (nil)       (nil)
check_heap after failed OOM       -1           3
```

Fixed by moving the `allocate_block` call above the epilogue write.

### `check_free_list` reports corruption on healthy heaps — open

`src/check_free_list.c:41`, in `detect_cycle`:

```c
if (*slow == *fast) {     /* slow and fast are char* — compares ONE BYTE */
```

This compares the low byte of each node's next-pointer rather than the pointers
themselves. Every payload is 16-aligned, so that byte has only 16 possible
values — roughly a 1-in-16 chance of a false "cycle detected" per step, which
makes a false positive near-certain on any non-trivial free list.

Randomized churn, 20 000 mixed malloc/free/realloc ops, heap checked every 512:

```
                       check_heap fails   check_free_list fails
current code                  0                    28
with `slow == fast`           0                     0
```

Zero across every seed with the one-character change. At the moment
`check_free_list()` returned `-1`, `check_heap()` reported 197 valid blocks and
`count_free_blocks_heap()` reported 48 free ones.

The unit tests never expose this because they only ever hold 0 or 1 free blocks.
It matters mainly because `check_free_list` is the instrument being used to hunt
the `nm` crash.

### Stress results (allocator itself, not compatibility)

| test | result |
|---|---|
| 16 threads, simultaneous first `malloc`, 300 runs | 0 corrupt, 0 crashes |
| 8 threads × 8000 mixed ops, 15 runs, payload tagged and verified | 0 corruption, `check_heap`=3 |
| 20 000 mixed ops × 8 seeds, single-threaded | 0 `check_heap` failures, 0 payload mismatches |

### Benchmark — the fixes cost nothing measurable

Re-measured on this machine, back to back, so the numbers are comparable to each
other but not to the round-1 figures in README.md (the baseline has drifted):

| build | mean | vs glibc |
|---|---|---|
| glibc | 12.803 s ± 0.320 | — |
| pre-fix (`origin/main`) | 15.418 s ± 0.774 | 1.20× |
| current | 15.851 s ± 0.480 | 1.24× |

2.8% apart with overlapping error bars. Isolating the new `merge_free_blocks`
call in `add_leftover_block` gave 14.284 s vs 14.614 s — also noise.

Memory cost is unchanged from round 1: 257 MB max RSS vs glibc's 229 MB (+12.1%),
minor faults 609 068 vs 589 201 (+3.4%). README.md's memory table still holds;
its **1.15× timing ratio now measures closer to 1.20–1.27×**.

---

# Round 3 — re-test after the `free()` `is_block_valid` guard (2026-08-05)

Scope: the two round-1 failures only (`nm`, `curl`). Nothing else was re-run.

Build: commit `25bb231`, `src/` and `include/` clean against HEAD, `make re`,
same flags as rounds 1 and 2 (`-fPIC -Werror -Wall -Wextra -fvisibility=hidden`,
no `-g`). Zero warnings under `-Werror`.
`md5sum libmalloc.so` = `405e066c28401e5b99b1e5697c13086b`. Same host as rounds 1–2.

## Harness — *full* level, stronger than round 2

```bash
env -u LD_PRELOAD "$@" >g.out 2>g.err;      rg=$?   # glibc
LD_PRELOAD=$LIB      "$@" >o.out 2>o.err;   ro=$?   # ours
# compared: exit code, stdout (cmp), stderr (cmp)
```

Back to round 1's standard: **stderr is diffed**, not merely captured. The command
is passed as an argv array, so there is no `eval`, no shell re-expansion, and no
`| md5sum` inside the measured command — a crash surfaces as an rc difference
rather than as a differing checksum.

## Results

| command | glibc | ours | verdict |
|---|---|---|---|
| `curl --version` | 0 | 0 | identical |
| `curl --help` | 0 | 0 | identical |
| `curl -s file:///etc/hostname` | 0 | 0 | identical |
| `nm tiny` | 0 | 0 | identical |
| `nm e.o` | 0 | 0 | identical |
| `nm /bin/ls` | 0 | 0 | identical |
| `nm -D /bin/ls` | 0 | 0 | identical |
| `nm -D /usr/lib64/libc.so.6` | 0 | 0 | identical |
| `nm -D /usr/lib64/libstdc++.so.6` | 0 | 0 | identical |
| `nm -C /usr/lib64/libstdc++.so.6` | 0 | 0 | identical |
| `nm -D /usr/lib64/libm.so.6` | 0 | 0 | identical |
| `nm -D /usr/lib64/libcrypto.so` | 0 | 0 | identical |
| `nm -D ./libmalloc.so` | 0 | 0 | identical |
| `nm --version` | 0 | 0 | identical |

14/14 identical on exit code, stdout **and** stderr. `e.o` was regenerated for this
round (`: > e.c && gcc -c e.c -o e.o`); it did not exist in the tree.

## Truncation check — the one that matters more than rc

The round-1/2 crash happened during `exit`, so stdout was lost even though the
symbol table had printed. Equal line counts, not just `rc=0`, is the pass condition.

| | glibc | ours | round 2 |
|---|---|---|---|
| `nm -D /usr/lib64/libc.so.6 \| wc -l` | 3078 | **3078** | 2989 (89 lines lost) |
| `nm -D /usr/lib64/libstdc++.so.6 \| wc -l` | 6274 | **6274** | — |

## Repeats — ×20 each under the preload

A fault inside an exit handler can depend on heap layout, so one clean run is not
evidence.

| command | failures |
|---|---|
| `nm tiny` | 0/20 |
| `curl --version` | 0/20 |
| `nm -D /usr/lib64/libstdc++.so.6` | 0/20 |

## Unit tests

`make tests_run` → **20/20 passing, 0 crashing**. Note this is 20, not the 21
recorded in `NEXT_STEPS.md`; the Makefile's `TSRC` currently builds four files
(`tests/malloc.c` 9, `tests/realloc.c` 6, `tests/calloc.c` 3,
`tests/malloc_usable_size.c` 2 = 20). The extra test in the 21 figure is not in
`TSRC` today. `tests/heap.c`, `tests/free_list.c`, `tests/churn_rand.c` and
`tests/thread_safety.c` exist in the tree but are not compiled by `tests_run`.

## What this round does *not* establish

- **The root cause is still unestablished.** `NEXT_STEPS.md` records that
  `is_block_valid` in `free()` fixes the *symptom*; nothing here identifies what
  overwrites the header with `"alysis"` in the first place. A guard that rejects a
  corrupt block still means a block got corrupted.
- **Only 14 commands were re-run.** The other ~84 in this file keep their round-1
  or round-2 status. Round 2's coverage gap (section above) is unchanged.
- Nothing here touches the three untested areas in `NEXT_STEPS.md` (`fork()` with
  the heap mutex held, `setrlimit` failure paths against the *current* `malloc.c` /
  `realloc.c`, sanitizer/valgrind runs).

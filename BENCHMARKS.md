# Benchmarks

hyperfine --warmup 1 -r 5 'make re' 'LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re'
Benchmark 1: make re
  Time (mean ± σ):     11.532 s ±  0.280 s    [User: 10.531 s, System: 0.925 s]
  Range (min … max):   11.256 s … 11.879 s    5 runs
 
Benchmark 2: LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re
  Time (mean ± σ):     14.191 s ±  0.331 s    [User: 12.992 s, System: 1.092 s]
  Range (min … max):   13.735 s … 14.643 s    5 runs
 
Summary
  make re ran
    1.23 ± 0.04 times faster than LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re



/usr/bin/time -v make re 2>&1 | grep -E 'Maximum resident|Minor|Major'
	Maximum resident set size (kbytes): 229312
	Major (requiring I/O) page faults: 0
	Minor (reclaiming a frame) page faults: 588874
[sephorahaniambossou@fedora B-OOP-400-PAR-4-1-tekspice-sephorah.aniambossou]$ /usr/bin/time -v env LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re 2>&1 | grep -E 'Maximum resident|Minor|Major'
	Maximum resident set size (kbytes): 257296
	Major (requiring I/O) page faults: 0
	Minor (reclaiming a frame) page faults: 608822



``
hyperfine --warmup 1 -r 5 'make re' 'LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re'
```

### Memory cost

`/usr/bin/time` is a tool used to measure the runtime and memory usage of a given program.

```
/usr/bin/time -v make re 2>&1 | grep -E 'Maximum resident|Minor|Major'
/usr/bin/time -v env LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re 2>&1 | grep -E 'Maximum resident|Minor|Major'
```





Benchmark 1: make re
  Time (mean ± σ):     11.668 s ±  0.345 s    [User: 10.665 s, System: 0.921 s]
  Range (min … max):   11.284 s … 12.191 s    5 runs
 
Benchmark 2: LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re
  Time (mean ± σ):     13.363 s ±  0.292 s    [User: 12.217 s, System: 1.057 s]
  Range (min … max):   13.064 s … 13.741 s    5 runs
 
Summary
  make re ran
    1.15 ± 0.04 times faster than LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re



[sephorahaniambossou@fedora nanotekspice]$ /usr/bin/time -v make re 2>&1 | grep -E 'Maximum resident|Minor|Major|User time|System time|Elapsed'
	User time (seconds): 10.00
	System time (seconds): 0.87
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:10.93
	Maximum resident set size (kbytes): 228996
	Major (requiring I/O) page faults: 0
	Minor (reclaiming a frame) page faults: 589052


[sephorahaniambossou@fedora nanotekspice]$ /usr/bin/time -v env LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re 2>&1 | grep -E 'Maximum resident|Minor|Major|User time|System time|Elapsed'
	User time (seconds): 11.67
	System time (seconds): 0.97
	Elapsed (wall clock) time (h:mm:ss or m:ss): 0:12.71
	Maximum resident set size (kbytes): 259028
	Major (requiring I/O) page faults: 2
	Minor (reclaiming a frame) page faults: 609646


--------------------------------
hyperfine --warmup 1 -r 5 -i  'ls / -ltRr' 'LD_PRELOAD=./libmalloc.so ls / -ltRr'
Benchmark 1: ls / -ltRr
  Time (mean ± σ):     17.675 s ±  0.034 s    [User: 4.793 s, System: 12.473 s]
  Range (min … max):   17.635 s … 17.710 s    5 runs
 
  Warning: Ignoring non-zero exit code.
 
Benchmark 2: LD_PRELOAD=./libmalloc.so ls / -ltRr
  Time (mean ± σ):     20.185 s ±  0.263 s    [User: 6.878 s, System: 12.882 s]
  Range (min … max):   19.872 s … 20.488 s    5 runs
 
  Warning: Ignoring non-zero exit code.
 
Summary
  ls / -ltRr ran
    1.14 ± 0.02 times faster than LD_PRELOAD=./libmalloc.so ls / -ltRr


hyperfine --warmup 1 -r 5 'make re' 'LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re'
Benchmark 1: make re
  Time (mean ± σ):     11.024 s ±  0.033 s    [User: 10.084 s, System: 0.885 s]
  Range (min … max):   10.991 s … 11.070 s    5 runs
 
Benchmark 2: LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re
  Time (mean ± σ):     13.065 s ±  0.083 s    [User: 11.982 s, System: 1.015 s]
  Range (min … max):   12.959 s … 13.188 s    5 runs
 
Summary
  make re ran
    1.19 ± 0.01 times faster than LD_PRELOAD=/home/sephorahaniambossou/delivery/quant/malloc/libmalloc.so make re

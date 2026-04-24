# LAB4 - MPI

## Exercise 1: Ring Communication

| Processes (p) | Time (Tp) sec | Speedup (Sp) | Efficiency (Ep) |
|---|---|---|---|
| 1 | 0.000056 | 1.00 | 100% |
| 2 | 0.000056 | 1.00 | 50% |
| 4 | 0.000293 | 0.19 | 4.8% |
| 6 | 0.000265 | 0.21 | 3.5% |

The Ring Communication program shows increased execution time as the number of processes increases, indicating that communication overhead becomes more significant. As more processes participate in the ring, each process spends additional time sending and receiving messages, which increases overall execution time. This overhead gradually dominates the computation, leading to performance degradation at higher process counts. With 4 processes, the execution time reaches 0.000293 seconds before slightly decreasing at 6 processes, showing that communication delays are a major bottleneck.

---

## Exercise 2: Parallel Array Sum

| Processes (p) | Time (Tp) sec | Speedup (Sp) | Efficiency (Ep) |
|---|---|---|---|
| 1 | 0.000007 | 1.00 | 100% |
| 2 | 0.000021 | 0.33 | 16.5% |
| 4 | 0.000067 | 0.10 | 2.5% |

The Parallel Array Sum exercise shows poor speedup even with just 2 processes. The efficiency drops to only 16.5% with 2 processes and further degrades to 2.5% with 4 processes. This happens because the amount of computation is very small relative to the overhead of distributing work, collecting results, and synchronizing processes. When the data size is too small, adding more processes actually makes the program slower because each process spends more time communicating than computing. The program could not run with 6 processes due to the array size not being divisible by 6.

---

## Exercise 3: Max-Min

| Processes (p) | Time (Tp) sec | Speedup (Sp) | Efficiency (Ep) |
|---|---|---|---|
| 1 | 0.000004 | 1.00 | 100% |
| 2 | 0.000024 | 0.17 | 8.5% |
| 4 | 0.000032 | 0.13 | 3.1% |
| 6 | 0.000120 | 0.03 | 0.55% |

Execution time increases significantly due to synchronization overhead among processes. In parallel programs, processes often need to coordinate and wait for each other at certain points, which introduces delays. This waiting time adds extra overhead and reduces overall performance. The Max-Min operation requires finding both the maximum and minimum values, which means processes must synchronize multiple times. With 6 processes, the execution time jumps to 0.00012 seconds, showing that synchronization overhead becomes extremely dominant. The efficiency at 6 processes is only 0.55%, demonstrating that this problem is not suitable for parallel execution with this many processes.

---

## Exercise 4: Parallel Dot Product

| Processes (p) | Time (Tp) sec | Speedup (Sp) | Efficiency (Ep) |
|---|---|---|---|
| 1 | 0.000007 | 1.00 | 100% |
| 2 | 0.000025 | 0.28 | 14% |
| 4 | 0.000063 | 0.11 | 2.75% |

Small data size leads to poor scalability because communication overhead becomes dominant compared to computation. Since the amount of work is limited, the time spent in communication between processes is relatively high. This reduces the benefits of parallel execution and results in very low performance improvement. With just 2 processes, the efficiency drops to 14%, and it gets even worse with 4 processes at 2.75%. The program could not run with 6 processes due to the vector size not being divisible by 6. This demonstrates that parallelizing problems with very small datasets is ineffective and can actually hurt performance significantly.

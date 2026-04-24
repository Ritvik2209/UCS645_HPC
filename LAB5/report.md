# LAB5

## Question 1: DAXPY MPI Implementation

| Processors | Execution Time (sec) | Speedup | Efficiency |
|------------|----------------------|---------|------------|
| 1          | 0.000242             | 1.000   | 1.000      |
| 2          | 0.000147             | 1.646   | 0.823      |
| 4          | 0.000094             | 2.574   | 0.644      |
| 6          | 0.000047             | 5.149   | 0.858      |

Execution time decreases significantly as the number of processors increases. The program performs the DAXPY operation (X[i] = a*X[i] + Y[i]) on vectors of size 2^16 across multiple processes. With 6 processors, we achieved 5.149x speedup compared to the single processor baseline. The efficiency remains reasonably good, showing that the parallelization overhead is minimal for this operation. The speedup is nearly linear because the workload is evenly divisible among all processes, and the computation time dominates over communication costs.

---

## Question 2: Broadcast Comparison

| Processes | MyBcast Time (s) | MPI_Bcast Time (s) |
|-----------|------------------|-------------------|
| 2         | 0.006000         | 0.001300          |
| 4         | 0.038000         | 0.003200          |
| 6         | 0.075000         | 0.007500          |

Custom broadcast shows a linear increase in execution time as the number of processes grows because data is sent sequentially from one process to the next. In contrast, MPI_Bcast performs much better as it uses a tree-based communication approach, where data is distributed in parallel across multiple processes. This reduces the number of communication steps and results in better performance. MPI_Bcast is significantly faster, being around 4.6x faster with 2 processes and over 10x faster with 6 processes, demonstrating the efficiency of MPI's built-in collective operations compared to manual point-to-point communication.

---

## Question 3: Distributed Dot Product

| Processes | Dot Product  | Time (sec) | Speedup | Efficiency |
|-----------|--------------|------------|---------|------------|
| 1         | 20000000     | 0.004500   | 1.000   | 1.000      |
| 2         | 20000000     | 0.002400   | 1.875   | 0.937      |
| 4         | 20000000     | 0.003200   | 1.406   | 0.351      |
| 6         | 19999990     | 0.002100   | 2.142   | 0.357      |

The initial speedup is good when moving from 1 processor to 2 processors, with an efficiency of 0.937. However, as the number of processes increases, the speedup begins to drop due to communication overhead and the limitations described by Amdahl's Law. A portion of the program remains sequential, which restricts the maximum achievable speedup, while additional communication between processes for broadcasting and reduction operations further reduces efficiency. The calculation of dot product on 500 million elements shows that while there is benefit to parallelization, the advantage diminishes as more processors are added.

---

## Question 4: Prime Number using MPI

This program uses a master-slave model in which the master process distributes numbers to worker processes for primality testing. Each slave process requests a number from the master, tests it for primality, and sends back the result. The master then distributes the next number to test. With 2 processes, the program took 0.521 seconds to find all primes up to 100. With 4 processes, it took 0.506 seconds, and with 6 processes, it took 0.561 seconds. The execution time remains relatively constant across different process counts because the problem size (finding primes up to 100) is small compared to the communication overhead. The dynamic load balancing ensures that worker processes don't remain idle, but with such a small range of numbers, adding more processes doesn't provide significant speedup.

---

## Question 5: Perfect Numbers using MPI

This program finds all perfect numbers up to 10,000 using a similar master-slave model. Each slave process requests a number from the master, checks if it's a perfect number by computing the sum of its divisors, and reports back the result. With 2 processes, the program took 0.480 seconds. With 4 processes, it took 0.507 seconds, and with 6 processes, it took 0.531 seconds. The execution times are consistent across different process counts, showing similar characteristics to the prime number program. The dynamic distribution of work allows processes to continue working until all numbers have been checked. However, the relatively small problem size (checking 10,000 numbers) means that communication overhead becomes significant when using more processes, leading to minimal improvement in execution time as more processes are added.

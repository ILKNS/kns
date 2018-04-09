- presume mempool needs more than one page(2 Mb)
- a queue to manage all free pages with two sentinels and a value to record free page num
- a overall page table to record page is free or allocated to which CPU
- per cpu page table to record page no. and occupied page num

virtual addr of 2Mb page:
|----12 bits----|----21 bits----|
     page no.     internal addr

33 bits needed so addr is uint64_t.

# kfifo
`using in-kernel kfifo.h to record overall free page`

# Ring
A lock-free queue derived from dpdk ring

- linux atomic operations
[linux doc/atomic_ops](https://www.kernel.org/doc/html/v4.14/core-api/atomic_ops.html)
- volatile is not couraged, but atomic_t is used.
- temp not using smp_mb around atomic operations, maybe leading to err
- sched_yield in kernel [](https://lwn.net/Articles/31462/)
- cpu_relax
- static inline
- cache aline in linux/cache.h
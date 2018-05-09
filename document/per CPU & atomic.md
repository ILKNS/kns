## Atomic Variable
Signed integer to keep atomic variable, which is used to substitute `volatile` declaration. `atomic_t` is simply a wrapper of number with specific API to visit the value in order to avoid wrong operations.  
In kns, `atomic_t` is adapted to declare counter or values with only one copy, `atomic_read` and `atomic_set` are frequently used. And CAS operation in ring is deployed with `atomic_cmpxchg`.  
Notice that, there's no barrier implied with atomic operations, some mb should be added to avoid incorrectness.

### Memory Barrier
- Memory Barriers are used as a way of intervening to instruct the compiler and the CPU to restrict the order.
- Memory barriers are only required when there 's a possibility of interaction between two CPUs or between a CPU and a device.
- There are minimal guarrantees provided by architectures.

  - Linux kernel memory barriers do not guarantee:
    - No guarantee that any of the memory accesses specified before a memory barrier will be complete by the completion of a memeory barrier instructions. Just a guarantee that the operations separated by the barrier will not interleave.
    - No guarantee that memory barrier on one CPU will have any direct effect on another CPU or any other hardware in the system.
    - No guarantee that a CPU will see the correct order of effects from a second CPU's accesses, even if the second CPU uses a memory barrier, unless the first CPU also uses a matching memory barrier.
    - No guarantee that some interveneing piece of off-the-CPU hardware will not reorder the memory accesses. **CPU cache coherency mechanisms should propagate the indirect effects of a memory barrier between CPUs, but might not do so in order** 

### Usage
`ATOMIC_INIT(i)` to declare when definition, and `atomic_set(v, i)` during runtime.  
Values should be touched with atomic_ops only.

## Local Variable
Some per cpu values are declared with `local_t`, which is just a wrapper of `atomic_long_t`. Variables should be touched with specific API's as well. They are declared as per cpu var with type `local_t`. And when visiting remote var, `smp_mb` is needed to keep sequence as well.

## Per Cpu Value
In ix, a lot of counter and data structures are declared per cpu to avoid extra locks. Generally, we use `get_cpu_var` and `put_cpu_var` pair to visit these variables with preemption safe context. In newest version of Linux API, API `this_xxx` with preemption operations are wrapped up, making it easier to be implemented.  
In ix, these things are rewroted, I just convert them to linux APIs.

## Reference
- [Linux Document: atomic variable](https://www.kernel.org/doc/html/v4.14/core-api/atomic_ops.html)
- [Linux Document: local variable](https://www.kernel.org/doc/html/v4.14/core-api/local_ops.html)
- [Linux code: percpu-defs.h](https://elixir.bootlin.com/linux/latest/source/include/linux/percpu-defs.h)

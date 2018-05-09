A lock-free ring buffer is implemented for buffer. 
Temperarily used for page allocation and restoration

## kfifo in Linux
A lock-free ring in linux, without lock when single producer and single consumer.  
Copy in the elements first, and then move the guard `in` and `out`. No length restriction for unsigned value `in` and `out`, keep growing 
in the procedure.

## ring in dpdk
However we need msmp ring for scalability, kfifo doesn't satisfy.  
Ring in dpdk is a higher perfomance msmp lock-free queue. It stores the pointers of elements.

### MP enqueue
One several cores, the guard are copied to local variables. Using CAS to keep the atomicity, while updating the queue seperately

## function porting
- atomic
  - atomic cas: `atomic_cmpxchg`
  - volatile is not safe, replace it with atomic_t
- cache align: `___cacheline_aligned`
- `cpu_relax` and `schedule` to hand over the threads

## Further
Maybe some lock in other functions could be replaced with RCU?

## Reference
- [kfifo api](https://lwn.net/Articles/347619/)
- [lock-free in linux](https://www.ibm.com/developerworks/cn/linux/l-cn-lockfree/)
- [rcu in linux](https://www.ibm.com/developerworks/cn/linux/l-rcu/)
- [ring in dpdk](https://dpdk.org/doc/guides/prog_guide/ring_lib.html)

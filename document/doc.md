# NIC driver
## Ideas
In the newest IX, DPDK's driver is used to initialize NIC, however, I did not figure out a way to adapt it into kns.
Originally derived from old version of DPDK's driver, and the newest ixgbe driver is refered to as I believe the newer the better.

## Layers
Two layers: the lower to function in phy layer, and the upper layer to rx/tx pkt. The upper layer is deprived from ixgbe driver, kns 
read and write ring directly.

## Ixgbe Driver
In the original ixgbe driver, `ixgbe_main.c` is used to hold functions to registe and remove driver. In DPDK, it is substitute with 

- First, restore the `ixgbe_probe` function as what PCI drivers do.
  - Detect pci dev, and check its info, all the `pci_` set of driver functions are brought back.
  - `adapter` and `netdev` are rewrited. In the original Linux, `netdev_ops` is used to hold callback functions to visit rings. However, we do not use vfs of network anymore.
  - iommap and pci mem allocation are also put here.
  - all the other lines are moved out to `ixgbe_init_adapter`, but in Linux, `probe` function finish all the initialization process.
- Callback Functions in ixgbe driver are also devided into two set:
  - `mac` for the upper layer, and `phy` for lower layer.
  - and `_general` holds all the general functions, and each specific file like `ixgbe_82599` hold different functions.
- In Linux, softirq is called to let the kernel jump to visit rings. We disable irq here, kernel poll the rings continuously.
- Finally, I compared all the driver functions in old version of ix and newest ixgbe driver:
  - some parameter's type and number of parameter are changed
  - some macros are disgarded and i do not know why, so i add them back
- The way to visit register of ixgbe is slightly different in linux and dpdk.
- Another hash function for rss is impled

### Files
- `ixgbe`: hold funcitons to register PCI dev and some general hardware functions
- `ixgbe_api`: some callback functions, just to be used as template for different type of ixgbe
- `ixgbe_common`: general functions of the driver, ixgbe register visit is also put here
- `ixgbe_dcb`: datacenter functions to config and calculate traffic
- `ixgbe_mbx` `ixgbe_pf` `ixgbe_vf`: some NIC functions
- `ixgbe_type`: important structures and macros
- `ixgbe_rxtx`: the file to provide stream for upper layer, the most important file
  - rx/tx queues are defined here
  - important rx/tx functions: rx queue poll, tx procedures
  - RSS set up
  - dcb configurations: while here, combined with some other configurations, make up the configuration of ixgbe

# Hugepage
## Ideas
- IX is trying to enhance performance by avoiding overheads from fine-grain resource management.
- Hugepage is stucked in memory avoiding swapping.
- larger page means less addr translation, thus reduces addr translation overhead and TLB misses.
- Reduce the L2 occupation for addr trans info.
- This buck of pages is used for mbuf storage, which is of similar size.

## Hugepage basics
- `libhugetlbfs` in user space is used to manage hugepage in Linux
- Typically, `mount` hugetlbfs first and setup the page size and numbers \
then, create a file under the fs we just created and `mmap` to get the address(the example in Linux Document)
- Or, we can just use `mmap` with flag `MAP_ANONYMOUS|MAP_HUGETLB` to get the address anonymously

## Transparent HugePage
Pages of a process are substituted by huge pages, and it is free to be splited into normal pages when huge page
is not available to be adapted here.
Hugepage here is represented in PMD level of page table, where needs compound pages.

### Compound page
Grouping two or more physically contiguous pages into a unit that can be treated as a single page, commonly used 
to create huge pages in `hugetlbfs` and `thp`. Allocating it with normal memory allocation function linke `alloc_pages()` 
with `__GFP_COMP` flag sets.

## Imple
Refered to [hugepage mark](nuncaalaprimera.com/2014/using-hugepage-backed-buffers-in-linux-kernel-driver)
And this is the way I tried. 
- Mount hugepagefs first
- Get the address of it with `mmap` and then send it to module with ioctl
- Get the physical address, and mark it with `get_user_pages_fast`(Note that this is frequently used in Linux v14 without barriers)

### [GFP_XXX](lists.kernelnewbies.org/pipermail/kernelnewbies/2012-August/005905)
Try passing GFP_KERNEL to __get_free_pages(), with the large contiguous number. 
Maybe an idea but met bugs

### THP
It looks the same but easier to be implemented with thp.

### notes
- 2MB temperarily for 4096 per node
- disable transparent huge page(maybe i need to modify it if thp is performed better)

## References
- IX
- [linux hugepage document](https://www.kernel.org/doc/Documentation/vm/hugetlbpage.txt)
- [Series introduction of Hugepage in lwn](https://lwn.net/Articles/374424/)
- [Transparent Hugepage](https://lwn.net/Articles/619738/)
- [4 level pagetbl in Linux](https://lwn.net/Articles/117749/)

# Atomic, Local and Per CPU variable
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

# Ring
A lock-free ring buffer is implemented for buffer. 
Temperarily used for page allocation and restoration

## kfifo in Linux
A lock-free ring in linux, without lock when single producer and single consumer.  
Copy in the elements first, and then move the guard `in` and `out`. No length restriction for unsigned value `in` and `out`, keep growing 
in the procedure.

## ring in dpdk
However we need msmp ring for scalability, kfifo doesn't satisfy.  
Ring in dpdk is a higher perfomance msmp lock-free queue. It stores the pointers of elements.

Similar with ordinary ring, we have `head` and `tail` ptr for both producer and consumer, and `next` to record guard move.

### MP enqueue
On each core, the guards are copied to local variables. Using CAS to keep the atomicity, while updating the element and guard seperately. Check the final state and update global guard at last.

Memory barrier and `cpu_relax` are used to keep correctness of sequence and give some time for other cores to operate.

## function porting
- atomic
  - atomic cas: `atomic_cmpxchg`
  - volatile is not safe, replace it with atomic_t
- cache align: `___cacheline_aligned`
- `cpu_relax` and `schedule` to hand over the core

## Further
Maybe some lock in other functions could be replaced with RCU?

## Reference
- [kfifo api](https://lwn.net/Articles/347619/)
- [lock-free in linux](https://www.ibm.com/developerworks/cn/linux/l-cn-lockfree/)
- [rcu in linux](https://www.ibm.com/developerworks/cn/linux/l-rcu/)
- [ring in dpdk](https://dpdk.org/doc/guides/prog_guide/ring_lib.html)

# Memory Manage
## IX Memory Manage
Devided by three parts `mem`, `page` and `mempool`. Memory address is translated to IX virtual address, and address translation is needed in the lowest layer of IX memory management.
- mem
  - Request anonymous huge page with `mmap` and flags `MAP_HUGETLB|MAP_HUGE_2MB` and bind it to specific numa node.
- page
  - Call function in mem to allocate contigous memory on node.
  - A per cpu value to record page refs and a page table to record ptr to virtual address.
  
Thus, actually, memory allocation and free are executed by linux syscalls. And memory isolation between different core is not implemented in this level actually.

- mempool
The actual per cpu isolation, a heap with fixed size memory without lock. A list to hold all mempool and `datastore` to store elements. Here, even the size of the element `mbuf` is restricted to 2Kb.

## KNS Memory Manage
As I am not able to allocate memory with `mmap`, I need to hold all huge pages at once in the lock free ring. Per cpu page table is implemented to hold the page number allocated to them, and the refs table to hold the reference number of each page.

No NUMA is available here, but I still left a empty layer for numa some day to maintain memory affinity.

The mempool is just a porting of it in IX.

# Timer
Higher resolution means higher performance
## Highres Timer In Linux
Linux provide timer with high resolution, and I read several codes of Linux driver find that the two main factor to adapt it
- The callback function: to be called after the time up.
- Hold the hrtimer in the structure bound with it which is easy.
- [highres timer API intro](https://lwn.net/Articles/167897/)

## Timer in IX
We provide a hierarchical timing wheel implementation for managing network timeouts, such as TCP retransmissions. It is optimized for the common case where most timers are canceled before they expire.  We support extremely high-resolution timeouts, as low as 16μs, which has been shown to improve performance during TCP incast congestion.

A timer event infrastructure
```
struct timer {
	struct hlist_node link;
	void (*handler)(struct timer *t, struct eth_fg *cur_fg);
	uint64_t expires;
	int fg_id;
};

struct timerwheel {
	uint64_t now_us;
	uint64_t timer_pos;
	struct hlist_head wheels[WHEEL_COUNT][WHEEL_SIZE];

};
```
In which timerwheel is declared per cpu variable.
And in delay procedure `rdtsc()` and `cpu_relax` are used to get cpu cycles and wait.

I checked the reimplementation of timer in IX, and only some functions need to be port to kernel as a wrapper of some functions.

# Flow Group
Adding or removing a thread, flow is migrated for load balancing. The migration is serialized and only one such process at any time for simplicity. 
The hrtimer is originally used here.
- Signals A to migrate `fgs` to B. A mark is used to distinguish it from normal ones, moving the packets from this group to remote one and stops 
all the timers belong to this group.
- Then A changed the RSS index to direct the flow to B. Thus the packet from thread A are lead to B.
- After that, when B receives the first packet from A, B signals A, remove the tag and add the timer to flow group.
![](https://github.com/ILKNS/kns/raw/master/document/fg.png)

# LWIP
Most part of lwip do not need any modification.
I just did some substitution:
- some API changed to linux kernel: ntol series, memory, string, per cpu variables
- TCP timer is changed to hrtimer in linux, some functions rewrote
- pkt in lwip is pbuf, replace it with mbuf

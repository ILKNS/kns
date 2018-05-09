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

## KNS Memory Mange
As I am not able to allocate memory with `mmap`, I need to hold all huge pages at once in the lock free ring. Per cpu page table is implemented to hold the page number allocated to them, and the refs table to hold the reference number of each page.

No NUMA is available here, but I still left a empty layer for numa some day to maintain memory affinity.

The mempool is just a porting of it in IX.

# Ideas
- IX is trying to enhance performance by avoiding overheads from fine-grain resource management.
- Hugepage is stucked in memory avoiding swapping.
- larger page means less addr translation, thus reduces addr translation overhead and TLB misses.
- Reduce the L2 occupation for addr trans info.
- This buck of pages is used for mbuf storage, which is of similar size.

# Hugepage basics
- `libhugetlbfs` in user space is used to manage hugepage in Linux
- Typically, `mount` hugetlbfs first and setup the page size and numbers \
then, create a file under the fs we just created and `mmap` to get the address(the example in Linux Document)
- Or, we can just use `mmap` with flag `MAP_ANONYMOUS|MAP_HUGETLB` to get the address anonymously

# Transparent HugePage

## [GFP_XXX](lists.kernelnewbies.org/pipermail/kernelnewbies/2012-August/005905)
Try passing GFP_KERNEL to __get_free_pages(), with the large contiguous number. 
Maybe an idea but met bugs

## Imple
Refered to [hugepage mark](nuncaalaprimera.com/2014/using-hugepage-backed-buffers-in-linux-kernel-driver)
And this is the way I tried. 
Mount hugepagefs first, get the address of it with `mmap` and then send it to module with ioctl \
get the physical address, and mark it with `get_user_pages_fast`(Note that this is frequently used in Linux v14 without barriers)

## notes
- 2MB temperarily for 4096 per node
- disable transparent huge page(maybe i need to modify it if thp is performed better)

## References
- IX
- [Series introduction of Hugepage in lwn](https://lwn.net/Articles/374424/)

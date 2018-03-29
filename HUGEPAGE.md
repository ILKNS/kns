#hugepage in kernel

## [lists.kernelnewbies.org/pipermail/kernelnewbies/2012-August/005905]
Try passing GFP_KERNEL to __get_free_pages(), with the large contiguous number.  
~But it is not huge page actually?~  

## [nuncaalaprimera.com/2014/using-hugepage-backed-buffers-in-linux-kernel-driver]

## notes
- 2MB temperarily for 4096 per node
- ban transparent huge page
- there's api in mmap to allocate hugepage
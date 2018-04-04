#include <kns/ioctl.h>

#include <linux/mm.h>
#include <linux/rwsem.h>
#include <linux/vmalloc.h>
/*
 * mmap_hp_u2k - map memory address from user to kernel
 */
void* mmap_hp_u2k(void* memaddr){
	struct page** pages;
	int retval, nid;
	unsigned long npages;
	unsigned long buffer_start = (unsigned long)memaddr;
	void* remapped;

	npages = 1 + (BUF_SIZE - 1)/PAGE_SIZE_2MB;

	pages = vmalloc(npages * sizeof(struct page *));

	retval = get_user_pages_fast(buffer_start, npages, 1, pages);

	nid = page_to_nid(pages[0]);
	remapped = vm_map_ram(pages, npages, nid, PAGE_KERNEL);

	return remapped;
}
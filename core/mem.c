/*
 * mem.c - memory management, only map hugepage from user to kernel here
 * to be modified if numa is used
 */

#include <kns/mem.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>

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

/*
 * Current Mapping Strategy:
 * 2 megabyte and 1 gigabyte pages grow down from MEM_PHYS_BASE_ADDR
 * 4 kilobyte pages are allocated from the standard mmap region
 */
static DEFINE_SPINLOCK(mem_lock);
static uintptr_t mem_pos = MEM_VIRTUAL_BASE_ADDR;

void *__mem_alloc_pages(void *base, int nr, int size,
			struct bitmask *mask, int numa_policy)
{
	return (void*)(0);
}

void *__mem_alloc_pages_onnode(void *base, int nr, int size, int node)
{
	return (void*)(0);
}

/**
 * mem_alloc_pages - allocates pages of memory
 * @nr: the number of pages to allocate
 * @size: the page size (4KB, 2MB, or 1GB)
 * @mask: the numa node mask
 * @numa_policy: the numa policy
 *
 * Returns a pointer (virtual address) to a page, or NULL if fail.
 */
void *mem_alloc_pages(int nr, int size, struct bitmask *mask, int numa_policy)
{
	return (void*)(0);
}

/**
 * mem_alloc_pages_onnode - allocates pages on a given numa node
 * @nr: the number of pages
 * @size: the page size (4KB, 2MB, or 1GB)
 * @numa_node: the numa node to allocate the pages from
 * @numa_policy: how strictly to take @numa_node
 *
 * Returns a pointer (virtual address) to a page or NULL if fail.
 */
void *mem_alloc_pages_onnode(int nr, int size, int node, int numa_policy)
{
	return (void*)(0);
}

/**
 * mem_free_pages - frees pages of memory
 * @addr: a pointer to the start of the pages
 * @nr: the number of pages
 * @size: the page size (4KB, 2MB, or 1GB)
 */
void mem_free_pages(void *addr, int nr, int size)
{
}

#define PAGEMAP_PGN_MASK	0x7fffffffffffffULL
#define PAGEMAP_FLAG_PRESENT	(1ULL << 63)
#define PAGEMAP_FLAG_SWAPPED	(1ULL << 62)
#define PAGEMAP_FLAG_FILE	(1ULL << 61)
#define PAGEMAP_FLAG_SOFTDIRTY	(1ULL << 55)

/**
 * mem_lookup_page_machine_addrs - determines the machine address of pages
 * @addr: a pointer to the start of the pages (must be @size aligned)
 * @nr: the number of pages
 * @size: the page size (4KB, 2MB, or 1GB)
 * @maddrs: a pointer to an array of machine addresses (of @nr elements)
 *
 * @maddrs is filled with each page machine address.
 *
 * Returns 0 if successful, otherwise failure.
 */
int mem_lookup_page_machine_addrs(void *addr, int nr, int size,
			          unsigned long *maddrs)
{
	return 0;
}


#pragma once

// replace numaid with cpuid

#include <asm/atomic.h>
#include <linux/percpu.h>
#include <linux/compiler.h>

#include <kns/mem.h>

#define NUM_PAGES 4096

struct page_ent {
	unsigned long maddr;
	atomic_t refcnt;
	uint32_t flags;
};

#define PAGE_NUM(addr) \
	PGN_2MB((uintptr_t) (addr) - (uintptr_t) MEM_VIRTUAL_BASE_ADDR)

#define	PAGE_FLAG_WILL_FREE	0x1
#define	PAGE_FLAG_CAN_FREE	0x2

extern struct page_ent page_tbl[];
DECLARE_PER_CPU(int32_t, page_refs[]);

#define VM_PERM_R	0x1

void page_init(void);

/**
 * is_page - determines an address is inside page memory
 * @addr: the address
 *
 * Returns true if the address is inside page memory.
 */
static inline bool is_page(void *addr)
{
	return ((uintptr_t) addr >= MEM_VIRTUAL_BASE_ADDR &&
		(uintptr_t) addr < MEM_VIRTUAL_END_ADDR);
}

/**
 * is_page_region - determines if an address range is inside page memory
 * @addr: the base address
 * @len: the length of the region
 *
 * Returns true if the region is inside page memory, otherwise false.
 */
static inline bool is_page_region(void *addr, size_t len)
{
	if (len > BUF_SIZE ||
	    (uintptr_t) addr < MEM_VIRTUAL_BASE_ADDR ||
	    (uintptr_t) addr + len > MEM_VIRTUAL_END_ADDR)
		return false;

	return true;
}

/**
 * page_machaddr - gets the machine address of a page
 * @addr: the address of (or in) the page
 *
 * NOTE: This variant is unsafe if a reference to the page is not already held.
 *
 * Returns the machine address of the page plus the offset within the page.
 */
static inline unsigned long page_machaddr(void *addr)
{
	/* return page_tbl[PAGE_NUM(addr)].maddr + PGOFF_2MB(addr); */
	return (unsigned long)((uintptr_t)addr + mem_phy_base);
}

/**
 * page_get - pins a memory page
 * @addr: the address of (or in) the page
 *
 * Returns the machine address of the page plus the offset within the page.
 */
static inline unsigned long page_get(void *addr)
{
	unsigned long idx = PAGE_NUM(addr);
	struct page_ent *ent = &page_tbl[idx];

	if (unlikely(ent->flags & PAGE_FLAG_WILL_FREE))
		atomic_inc(&ent->refcnt);
	else
		this_cpu_inc(page_refs[idx]);

	return ent->maddr + PGOFF_2MB(addr);
}

extern void __page_put_slow(void *addr);

/**
 * page_put - unpins an iomap memory page
 * @addr: the address of (or in) the page
 */
static inline void page_put(void *addr)
{
	unsigned long idx = PAGE_NUM(addr);
	struct page_ent *ent = &page_tbl[idx];

	if (unlikely(ent->flags & PAGE_FLAG_WILL_FREE))
		__page_put_slow(addr);
	else
		this_cpu_dec(page_refs[idx]);
}

/**
 * page_alloc - allocate pages
 * @nr: the number of pages to allocate
 *
 * Returns an address, or NULL if fail.
 */
void **page_alloc(unsigned int nr);

int page_free_from_node(void **pages, unsigned int nr, int cpu);

void page_free(void **phy_addr, unsigned int nr);

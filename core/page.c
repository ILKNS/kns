#include <kns/page.h>
#include <kns/ring.h>

#include <linux/mm.h>
#include <linux/rwsem.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

struct page_ent page_tbl[NUM_PAGES];
DEFINE_PER_CPU(int32_t, page_refs[NUM_PAGES]);
DEFINE_PER_CPU(uint32_t, page_tbl_cpu[NUM_PAGES]);
DEFINE_PER_CPU(int, page_num_cpu);
static int page_no[NUM_PAGES];

static atomic64_t pagenum_free = ATOMIC64_INIT(NUM_PAGES);
static struct ring* free_page_ring;

void page_init(void){
	char* name = "free_page_ring";
	void** pages;
	int i;

	pages = kzalloc(NUM_PAGES*sizeof(void*), GFP_KERNEL);

	/* create a ring with mp&mc */
	free_page_ring = ring_create(name, NUM_PAGES, -1, 0);
	for(i = 0; i < NUM_PAGES; i++){
		page_no[i] = i;
		pages[i] = &page_no[i];
	}
	ring_enqueue_bulk(free_page_ring, pages, NUM_PAGES);
}

static inline struct page_ent *addr_to_page_ent(void *addr)
{
	return &page_tbl[PAGE_NUM(addr)];
}

/**
 * __page_put_slow - the slow path for decrementing page refernces
 * @addr: the address
 *
 * This function actually frees the page (if possible).
 */
void __page_put_slow(void *addr)
{
	bool no_refs;
	struct page_ent *ent = addr_to_page_ent(addr);

	no_refs = atomic_dec_and_test(&ent->refcnt);

	/* can we free the page yet? */
	if (!no_refs || !(ent->flags & PAGE_FLAG_CAN_FREE))
		return;

	/* do nothing, keep the page in kernel */
	//mem_free_page((void *) PGADDR_2MB(addr), PGSIZE_2MB);
	atomic64_inc(&pagenum_free);
}

/**
 * page_alloc_contig_on_node - allocates a guest-physically set of 2MB pages
 * @nr: the number of pages
 * @cpu: the cpu id
 *
 * Returns an table of page number, or NULL if fail.
 */
static int page_alloc_on_node(void **pages, unsigned int nr)
{
	int ret, i;
	int* page_tbl_update;

	ret = ring_dequeue_bulk(free_page_ring, pages, nr);
	BUG_ON(ret == 0);

	page_tbl_update = this_cpu_ptr(page_tbl_cpu);
	
	for(i = 0; i < nr; i++)
		page_tbl_update[*(int*)pages[i]]++;

	__this_cpu_add(page_num_cpu, nr);

	atomic64_add(nr, &pagenum_free);

	return 0;
}

int page_free_from_node(void **pages, unsigned int nr, int cpu)
{
	int ret, i, ref;
	int *page_tbl_update;

	ret = ring_enqueue_bulk(free_page_ring, pages, nr);
	BUG_ON(ret == 0);

	preempt_disable();
	if(cpu != smp_processor_id()){
		printk(KERN_ERR"page_alloc_on_node: cpu migrated.");
		preempt_enable();
		return -1; 
	}
	page_tbl_update = this_cpu_ptr(page_tbl_cpu);
	for(i = 0; i < nr; i++){
		ref = page_tbl_update[*(int*)pages[i]]--;
		BUG_ON(ref == 0);
	}

	__this_cpu_sub(page_num_cpu, nr);
	preempt_enable();

	return 0;
}

void **page_alloc(unsigned int nr)
{
	int ret;
	void** phy_addrs;
	void* page_index[nr];

	preempt_disable();
	ret = page_alloc_on_node(page_index, nr);
	preempt_enable();

	BUG_ON(ret == 0);

	return phy_addrs;
}
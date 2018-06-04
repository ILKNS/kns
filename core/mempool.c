/*
 * Mempool.c - A fast per-CPU memory pool implementation
 *
 * A mempool is a heap that only supports allocations of a
 * single fixed size. Generally, mempools are created up front
 * with non-trivial setup cost and then used frequently with
 * very low marginal allocation cost.
 * 
 * Mempools are not thread-safe. This allows them to be really
 * efficient but you must provide your own locking if you intend
 * to use a memory pool with more than one core.
 *
 * Mempools rely on a data store -- which is where the allocation
 * happens.  The backing store is thread-safe, meaning that you
 * typically have datastore per NUMA node and per data type.
 *
 * Mempool datastores also
 * have NUMA affinity to the core they were created for, so
 * using them from the wrong core frequently could result in
 * poor performance.
 *
 * The basic implementation strategy is to partition a region of
 * memory into several fixed-sized slots and to maintain a
 * singly-linked free list of available slots. Objects are
 * allocated in LIFO order so that preference is given to the
 * free slots that are most cache hot.
 */

#include <kns/mempool.h>
#include <kns/page.h>
// #include <ix/vm.h>

static struct mempool_datastore *mempool_all_datastores;
#ifdef ENABLE_KSTATS
static struct timer mempool_timer;
#endif
/**
 * mempool_alloc_2  -- second stage allocator; may spinlock
 * @m: mempool
 */

void *mempool_alloc_2(struct mempool *m)
{

	struct mempool_hdr *h;
	struct mempool_datastore *mds;
	unsigned long flags;
	BUG_ON (m->magic == MEMPOOL_MAGIC);
	BUG_ON (m->head == NULL);
	
	if (m->private_chunk) {
		h = m->private_chunk;
		m->head = h->next;
		m->private_chunk = NULL;
		return h;
	}

	mds = m->datastore;

	BUG_ON(mds);
	spin_lock_irqsave(&mds->lock, flags);
	h = mds->chunk_head;
	if (likely(h)) {
		mds->chunk_head = h->next_chunk;
		m->head = h->next;
		mds->free_chunks--;
		mds->num_locks++;
	}
	spin_unlock_irqrestore(&mds->lock,flags);
#ifdef DEBUG_MEMPOOL
	struct mempool_hdr *cur = h;
	for (;cur;cur=cur->next) {
		struct mempool **hidden = (struct mempool **)cur;
		hidden[-1] = m;
	}
#endif
	return h;

}

/**
 * mempool_free_2 -- second stage free
 * @m: mempool
 * @ptr: ptr
 */

void mempool_free_2(struct mempool *m, void *ptr)
{
	struct mempool_hdr *elem = (struct mempool_hdr *) ptr;
	unsigned long flags;
	BUG_ON (m->num_free == m->chunk_size);

	elem->next = NULL;

	if (m->private_chunk != NULL) {
		struct mempool_datastore *mds = m->datastore;
		spin_lock_irqsave(&mds->lock, flags);
		m->private_chunk->next_chunk = mds->chunk_head;
		mds->chunk_head = m->private_chunk;
		mds->free_chunks++;
		mds->num_locks++;
		spin_unlock_irqrestore(&mds->lock, flags);
	}
	m->private_chunk = m->head;
	m->head = elem;
	m->num_free = 1;
}


/**
 * mempool_init_buf_with_pages - creates the object and puts them in the doubly-linked list
 * @m: datastore
 *
 */
int mempool_init_buf_with_pages(struct mempool_datastore *mds, int elems_per_page, int nr_pages,
			    size_t elem_len)
{
	int i, j, chunk_count=0;
	struct mempool_hdr *cur, *head=NULL, *prev=NULL;

	for (i = 0; i < nr_pages; i++) {
		cur = (struct mempool_hdr *)
			((uintptr_t) mds->buf + i * PAGE_SIZE_2MB + MEMPOOL_INITIAL_OFFSET);
		for (j = 0; j < elems_per_page; j++) {
			if (prev==NULL)
				head= cur;
			else
				prev->next = cur;

			chunk_count++;
			if (chunk_count == mds->chunk_size) {
				if (mds->chunk_head == NULL) {
					mds->chunk_head = head;
				} else {
					head->next_chunk = mds->chunk_head;
					mds->chunk_head = head;
				}
				head = NULL;
				prev = NULL;
				chunk_count = 0;
				mds->num_chunks++;
				mds->free_chunks++;
			} else {
				prev = cur;
			}
			cur = (struct mempool_hdr *)
				((uintptr_t) cur + elem_len);
		}
	}

	return 0;
}


/**
 * mempool_create_datastore - initializes a memory pool datastore
 * @nr_elems: the minimum number of elements in the total pool
 * @elem_len: the length of each element
 * @nostraddle: (bool) 1 == objects cannot straddle 2MB pages
 * @chunk_size: the number of elements in a chunk (allocated to a mempool)
 *
 * NOTE: mempool_createdatastore() will create a pool with at least @nr_elems,
 * but possibily more depending on page alignment.
 *
 * There should be one datastore per C data type (in general).  
 * Each core, flow-group or unit of concurrency will create a distinct mempool leveraging the datastore
 * 
 * Returns 0 if successful, otherwise fail.
 */

int mempool_create_datastore(struct mempool_datastore *mds, int nr_elems, size_t elem_len, int nostraddle, int chunk_size, const char *name)
{
	int nr_pages, elems_per_page;

	BUG_ON (mds->magic == 0);
	BUG_ON ((chunk_size & (chunk_size-1))==0);
	BUG_ON (((nr_elems / chunk_size) * chunk_size)==nr_elems);


	if (!elem_len || !nr_elems)
		return -EINVAL;

	mds->magic = MEMPOOL_MAGIC;
	mds->prettyname = name;
	elem_len = ALIGN(elem_len, sizeof(long)) + MEMPOOL_INITIAL_OFFSET;

	//if (nostraddle) {
		elems_per_page = PAGE_SIZE_2MB / elem_len;
		nr_pages = DIV_ROUND_UP(nr_elems, elems_per_page);
		mds->buf = page_alloc(nr_pages);
		BUG_ON(mds->buf);
	//} else {
		// preserved here, mbuf is less than 2MB now
		// nr_pages = PGN_2MB(nr_elems * elem_len + PGMASK_2MB);
		// nr_elems = nr_pages * PAGE_SIZE_2MB / elem_len;
		// mds->buf = mem_alloc_pages(nr_pages, PAGE_SIZE_2MB, NULL, MPOL_PREFERRED);
	//}

	mds->nr_pages = nr_pages;
	mds->nr_elems = nr_elems;
	mds->elem_len = elem_len;
	mds->chunk_size = chunk_size;
	mds->nostraddle = nostraddle;

	spin_lock_init(&mds->lock);

	if (mds->buf == MAP_FAILED || mds->buf == 0) {
		printk(KERN_ERR"mempool alloc failed\n");
		printk(KERN_ERR"Unable to create mempool datastore %s\n",name);

		printk(KERN_ERR"unable to create mempool datstore for %s\n",name);
		return -ENOMEM;
	}

	if (nostraddle) {
		int elems_per_page = PAGE_SIZE_2MB / elem_len;
		mempool_init_buf_with_pages(mds, elems_per_page, nr_pages, elem_len);
	} else
		mempool_init_buf_with_pages(mds, nr_elems, 1, elem_len);

	mds->next_ds = mempool_all_datastores;
	mempool_all_datastores = mds;

	printk(KERN_ERR"mempool_datastore: %-15s pages:%4u elem_len:%4lu nostraddle:%d chunk_size:%d num_chunks:4%d\n",
	       name,
	       nr_pages,
	       mds->elem_len, mds->nostraddle,mds->chunk_size, mds->num_chunks);

	return 0;
}


/**
 * mempool_create - initializes a memory pool
 * @nr_elems: the minimum number of elements in the pool
 * @elem_len: the length of each element
 *
 * NOTE: mempool_create() will create a pool with at least @nr_elems,
 * but possibily more depending on page alignment.
 *
 * Returns 0 if successful, otherwise fail.
 */
int kns_mempool_create(struct mempool *m, struct mempool_datastore *mds, int16_t sanity_type, int16_t sanity_id)
{
	if (mds->magic != MEMPOOL_MAGIC)
		printk(KERN_ERR"mempool_create when datastore does not exist\n");

	BUG_ON(mds->magic == MEMPOOL_MAGIC);
	
	if (m->magic != 0)
		printk(KERN_ERR"mempool_create attempt to call twice (ds=%s)\n",mds->prettyname);

	BUG_ON(m->magic == 0);
	m->magic = MEMPOOL_MAGIC;
	m->buf = mds->buf;
	m->datastore = mds;
	m->head = NULL;
	m->sanity = (sanity_type <<16) | sanity_id;
	m->nr_elems = mds->nr_elems;
	m->elem_len = mds->elem_len;
	m->nostraddle = mds->nostraddle;
	m->chunk_size = mds->chunk_size;
	m->iomap_addr = mds->iomap_addr;
	m->iomap_offset = mds->iomap_offset;
	return 0;
}

/**
 * mempool_destroy - cleans up a memory pool and frees memory
 * @m: the memory pool
 */ 
void mempool_destroy_datastore(struct mempool_datastore *mds)
{
	page_free(mds->buf, mds->nr_pages);
	mds->buf = NULL;
	mds->chunk_head = NULL;
	mds->magic = 0;
}

/**
 * mempool_pagemem_map_to_user - make the memory pool available to user memory
 * @m: the memory pool.
 *
 * NOTE: we map the memory read-only.
 *
 * Returns 0 if successful, otherwise fail.
 */
// int mempool_pagemem_map_to_user(struct mempool_datastore *m)
// {
// 	m->iomap_addr = vm_map_to_user(m->buf, m->nr_pages,
// 				       PAGE_SIZE_2MB, VM_PERM_R);
// 	if (!m->iomap_addr)
// 		return -ENOMEM;

// 	m->iomap_offset = (uintptr_t) m->iomap_addr - (uintptr_t) m->buf;
// 	return 0;
// }

/**
 * mempool_pagemem_destroy - destroys a memory pool allocated with page memory
 * @m: the memory pool
 */
void mempool_pagemem_destroy(struct mempool_datastore *m)
{
	// if (m->iomap_addr) {
	// 	vm_unmap(m->iomap_addr, m->nr_pages, PAGE_SIZE_2MB);
	// 	m->iomap_addr = NULL;
	// }

    page_free(m->buf, m->nr_pages);
    m->buf = NULL;
    m->chunk_head = NULL;
}

int mempool_init(void)
{
#ifdef ENABLE_KSTATS
	timer_init_entry(&mempool_timer, mempool_printstats);
	timer_add(&mempool_timer, NULL, PRINT_INTERVAL);
#endif
	return 0;
}


#include <kns/ring.h>

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>

/* true if x is a power of 2 */
#define POWEROF2(x) ((((x)-1) & (x)) == 0)

/* return the size of memory occupied by a ring */
ssize_t
ring_get_memsize(unsigned count)
{
	ssize_t sz;

	/* count must be a power of 2 */
	if ((!POWEROF2(count)) || (count > RING_SZ_MASK )) {
		printk(KERN_ERR
			"Ring: Requested size is invalid, must be power of 2, and "
			"do not exceed the size limit %u\n", RING_SZ_MASK);
		return -EINVAL;
	}

	sz = sizeof(struct ring) + count * sizeof(void *);
	sz = L1_CACHE_ALIGN(sz);
	return sz;
}

int
ring_init(struct ring *r, const char *name, unsigned count,
	unsigned flags)
{
	/* compilation-time checks */
	BUG_ON((sizeof(struct ring) & L1_CACHE_BYTES) != 0);
	BUG_ON((offsetof(struct ring, cons) & L1_CACHE_BYTES) != 0);
	BUG_ON((offsetof(struct ring, prod) & L1_CACHE_BYTES) != 0);

	/* init the ring structure */
	memset(r, 0, sizeof(*r));
	snprintf(r->name, sizeof(r->name), "%s", name);
	r->flags = flags;
	r->prod.watermark = count;
	r->prod.sp_enqueue = !!(flags & RING_F_SP_ENQ);
	r->cons.sc_dequeue = !!(flags & RING_F_SC_DEQ);
	r->prod.size = r->cons.size = count;
	r->prod.mask = r->cons.mask = count-1;
	atomic_set(&r->prod.head, 0);
	atomic_set(&r->cons.head, 0);
	atomic_set(&r->prod.tail, 0);
	atomic_set(&r->cons.tail, 0);

	return 0;
}

/* create the ring */
struct ring *
ring_create(const char *name, unsigned count, int socket_id,
		unsigned flags)
{
	struct ring *r;
	ssize_t ring_size;

	ring_size = ring_get_memsize(count);
	if (ring_size < 0)
		return NULL;

	/* reserve a memory zone for this ring. If we can't get rte_config or
	 * we are secondary process, the memzone_reserve function will set
	 * rte_errno for us appropriately - hence no check in this this function */
	r = kzalloc(ring_size, GFP_KERNEL);
	if (r != NULL)
		/* no need to check return value here, we already checked the
		 * arguments above */
		ring_init(r, name, count, flags);
	else {
		r = NULL;
		printk(KERN_ERR"Ring:Cannot reserve memory\n");
	}

	return r;
}

/* free the ring */
void
ring_free(struct ring *r)
{
	if (r == NULL)
		return;

	kfree(r);
	r = NULL;
}

/*
 * change the high water mark. If *count* is 0, water marking is
 * disabled
 */
int
ring_set_water_mark(struct ring *r, unsigned count)
{
	if (count >= r->prod.size)
		return -EINVAL;

	/* if count is 0, disable the watermarking */
	if (count == 0)
		count = r->prod.size;

	r->prod.watermark = count;
	return 0;
}


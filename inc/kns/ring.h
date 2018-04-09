#pragma once

#include <asm/processor.h>
#include <linux/atomic.h>
#include <linux/cache.h>
#include <linux/compiler.h>
#include <linux/sched.h>
#include <linux/types.h>

/**
 * @file
 * Ring, derived from dpdk/rte_ring
 *
 * The Ring Manager is a fixed-size queue, implemented as a table of
 * pointers. Head and tail pointers are modified atomically, allowing
 * concurrent access to it. It has the following features:
 *
 * - FIFO (First In First Out)
 * - Maximum size is fixed; the pointers are stored in a table.
 * - Lockless implementation.
 * - Multi- or single-consumer dequeue.
 * - Multi- or single-producer enqueue.
 * - Bulk dequeue.
 * - Bulk enqueue.
 *
 * Note: the ring implementation is not preemptable. A lcore must not
 * be interrupted by another task that uses the same ring.
 *
 */

enum ring_queue_behavior {
	RING_QUEUE_FIXED = 0, /* Enq/Deq a fixed number of items from a ring */
	RING_QUEUE_VARIABLE   /* Enq/Deq as many items a possible from ring */
};

#define RING_NAMESIZE 32 /**< The maximum length of a ring name. */
#define RING_PAUSE_REP_COUNT 0 /**< Yield after pause num of times, no yield
                                    *   if RING_PAUSE_REP not defined. */

/**
 * An ring structure.
 *
 * The producer and the consumer have a head and a tail index. The particularity
 * of these index is that they are not between 0 and size(ring). These indexes
 * are between 0 and 2^32, and we mask their value when we access the ring[]
 * field. Thanks to this assumption, we can do subtractions between 2 index
 * values in a modulo-32bit base: that's why the overflow of the indexes is not
 * a problem.
 */
struct ring {
	char name[RING_NAMESIZE];    /**< Name of the ring. */
	int flags;                       /**< Flags supplied at creation. */

	/** Ring producer status. */
	struct prod {
		uint32_t watermark;      /**< Maximum items before EDQUOT. */
		uint32_t sp_enqueue;     /**< True, if single producer. */
		uint32_t size;           /**< Size of ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		atomic_t head;			 /**< Producer head. */
		atomic_t tail; 			 /**< Producer tail. */
	} prod ____cacheline_aligned;

	/** Ring consumer status. */
	struct cons {
		uint32_t sc_dequeue;     /**< True, if single consumer. */
		uint32_t size;           /**< Size of the ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		atomic_t head;  		 /**< Consumer head. */
		atomic_t tail; 			 /**< Consumer tail. */
	} cons ____cacheline_aligned;

	void * ring[0] ____cacheline_aligned; /**< Memory space of ring starts here.
	                                       * not volatile so need to be careful
	                                       * about compiler re-ordering */
};

#define RING_F_SP_ENQ 0x0001 /**< The default enqueue is "single-producer". */
#define RING_F_SC_DEQ 0x0002 /**< The default dequeue is "single-consumer". */
#define RING_QUOT_EXCEED (1 << 31)  /**< Quota exceed for burst ops */
#define RING_SZ_MASK  (unsigned)(0x0fffffff) /**< Ring size mask */

/**
 * Calculate the memory size needed for a ring
 *
 * This function returns the number of bytes needed for a ring, given
 * the number of elements in it. This value is the sum of the size of
 * the structure ring and the size of the memory needed by the
 * objects pointers. The value is aligned to a cache line size.
 *
 * @param count
 *   The number of elements in the ring (must be a power of 2).
 * @return
 *   - The memory size needed for the ring on success.
 *   - -EINVAL if count is not a power of 2.
 */
ssize_t ring_get_memsize(unsigned count);

/**
 * Initialize a ring structure.
 *
 * Initialize a ring structure in memory pointed by "r". The size of the
 * memory area must be large enough to store the ring structure and the
 * object table. It is advised to use ring_get_memsize() to get the
 * appropriate size.
 *
 * The ring size is set to *count*, which must be a power of two. Water
 * marking is disabled by default. The real usable ring size is
 * *count-1* instead of *count* to differentiate a free ring from an
 * empty ring.
 *
 * The ring is not added in RTE_TAILQ_RING global list. Indeed, the
 * memory given by the caller may not be shareable among dpdk
 * processes.
 *
 * @param r
 *   The pointer to the ring structure followed by the objects table.
 * @param name
 *   The name of the ring.
 * @param count
 *   The number of elements in the ring (must be a power of 2).
 * @param flags
 *   An OR of the following:
 *    - RING_F_SP_ENQ: If this flag is set, the default behavior when
 *      using ``ring_enqueue()`` or ``ring_enqueue_bulk()``
 *      is "single-producer". Otherwise, it is "multi-producers".
 *    - RING_F_SC_DEQ: If this flag is set, the default behavior when
 *      using ``ring_dequeue()`` or ``ring_dequeue_bulk()``
 *      is "single-consumer". Otherwise, it is "multi-consumers".
 * @return
 *   0 on success, or a negative value on error.
 */
int ring_init(struct ring *r, const char *name, unsigned count,
	unsigned flags);

/**
 * Create a new ring named *name* in memory.
 *
 * This function uses ``how`` to allocate memory. Then it
 * calls ring_init() to initialize an empty ring.
 *
 * The new ring size is set to *count*, which must be a power of
 * two. Water marking is disabled by default. The real usable ring size
 * is *count-1* instead of *count* to differentiate a free ring from an
 * empty ring.
 *
 * The ring is added in RTE_TAILQ_RING list.
 *
 * @param name
 *   The name of the ring.
 * @param count
 *   The size of the ring (must be a power of 2).
 * @param socket_id
 *   The *socket_id* argument is the socket identifier in case of
 *   NUMA. The value can be *SOCKET_ID_ANY* if there is no NUMA
 *   constraint for the reserved zone.
 * @param flags
 *   An OR of the following:
 *    - RING_F_SP_ENQ: If this flag is set, the default behavior when
 *      using ``ring_enqueue()`` or ``ring_enqueue_bulk()``
 *      is "single-producer". Otherwise, it is "multi-producers".
 *    - RING_F_SC_DEQ: If this flag is set, the default behavior when
 *      using ``ring_dequeue()`` or ``ring_dequeue_bulk()``
 *      is "single-consumer". Otherwise, it is "multi-consumers".
 * @return
 *   On success, the pointer to the new allocated ring. NULL on error with
 *    errno set appropriately. Possible errno values include:
 *    - E_RTE_NO_CONFIG - function could not get pointer to rte_config structure
 *    - E_RTE_SECONDARY - function was called from a secondary process instance
 *    - EINVAL - count provided is not a power of 2
 *    - ENOSPC - the maximum number of memzones has already been allocated
 *    - EEXIST - a memzone with the same name already exists
 *    - ENOMEM - no appropriate memory area found in which to create memzone
 */
struct ring *ring_create(const char *name, unsigned count,
	int socket_id, unsigned flags);

/**
 * De-allocate all memory used by the ring.
 *
 * @param r
 *   Ring to free
 */
void ring_free(struct ring *r);

/**
 * Change the high water mark.
 *
 * If *count* is 0, water marking is disabled. Otherwise, it is set to the
 * *count* value. The *count* value must be greater than 0 and less
 * than the ring size.
 *
 * This function can be called at any time (not necessarily at
 * initialization).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param count
 *   The new water mark value.
 * @return
 *   - 0: Success; water mark changed.
 *   - -EINVAL: Invalid water mark value.
 */
int ring_set_water_mark(struct ring *r, unsigned count);

/* the actual enqueue of pointers on the ring.
 * Placed here since identical code needed in both
 * single and multi producer enqueue functions */
#define ENQUEUE_PTRS() do { \
	const uint32_t size = r->prod.size; \
	uint32_t idx = prod_head & mask; \
	if (likely(idx + n < size)) { \
		for (i = 0; i < (n & ((~(unsigned)0x3))); i+=4, idx+=4) { \
			r->ring[idx] = obj_table[i]; \
			r->ring[idx+1] = obj_table[i+1]; \
			r->ring[idx+2] = obj_table[i+2]; \
			r->ring[idx+3] = obj_table[i+3]; \
		} \
		switch (n & 0x3) { \
			case 3: r->ring[idx++] = obj_table[i++]; \
			case 2: r->ring[idx++] = obj_table[i++]; \
			case 1: r->ring[idx++] = obj_table[i++]; \
		} \
	} else { \
		for (i = 0; idx < size; i++, idx++)\
			r->ring[idx] = obj_table[i]; \
		for (idx = 0; i < n; i++, idx++) \
			r->ring[idx] = obj_table[i]; \
	} \
} while(0)

/* the actual copy of pointers on the ring to obj_table.
 * Placed here since identical code needed in both
 * single and multi consumer dequeue functions */
#define DEQUEUE_PTRS() do { \
	uint32_t idx = cons_head & mask; \
	const uint32_t size = r->cons.size; \
	if (likely(idx + n < size)) { \
		for (i = 0; i < (n & (~(unsigned)0x3)); i+=4, idx+=4) {\
			obj_table[i] = r->ring[idx]; \
			obj_table[i+1] = r->ring[idx+1]; \
			obj_table[i+2] = r->ring[idx+2]; \
			obj_table[i+3] = r->ring[idx+3]; \
		} \
		switch (n & 0x3) { \
			case 3: obj_table[i++] = r->ring[idx++]; \
			case 2: obj_table[i++] = r->ring[idx++]; \
			case 1: obj_table[i++] = r->ring[idx++]; \
		} \
	} else { \
		for (i = 0; idx < size; i++, idx++) \
			obj_table[i] = r->ring[idx]; \
		for (idx = 0; i < n; i++, idx++) \
			obj_table[i] = r->ring[idx]; \
	} \
} while (0)

/**
 * @internal Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @param behavior
 *   RING_QUEUE_FIXED:    Enqueue a fixed number of items from a ring
 *   RING_QUEUE_VARIABLE: Enqueue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RING_QUEUE_FIXED
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 *   if behavior = RING_QUEUE_VARIABLE
 *   - n: Actual number of objects enqueued.
 */
static inline int __attribute__((always_inline))
__ring_mp_do_enqueue(struct ring *r, void * const *obj_table,
	unsigned n, enum ring_queue_behavior behavior)
{
	uint32_t prod_head, prod_next;
	uint32_t cons_tail, free_entries;
	const unsigned max = n;
	int success;
	unsigned i, rep = 0;
	uint32_t mask = r->prod.mask;
	int ret;

	/* Avoid the unnecessary cmpset operation below, which is also
	 * potentially harmful when n equals 0. */
	if (n == 0)
		return 0;

	/* move prod.head atomically */
	do {
		/* Reset n to the initial burst count */
		n = max;

		prod_head = atomic_read(&r->prod.head);
		cons_tail = atomic_read(&r->cons.tail);
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * prod_head > cons_tail). So 'free_entries' is always between 0
		 * and size(ring)-1. */
		free_entries = (mask + cons_tail - prod_head);

		/* check that we have enough room in ring */
		if (unlikely(n > free_entries)) {
			if (behavior == RING_QUEUE_FIXED)
				return -ENOBUFS;
			else {
				/* No free entry available */
				if (unlikely(free_entries == 0))
					return 0;
				n = free_entries;
			}
		}

		prod_next = prod_head + n;
		success = atomic_cmpxchg(&r->prod.head, prod_head,
					      prod_next);
	} while (unlikely(success == 0));

	/* write entries in ring */
	ENQUEUE_PTRS();
	smp_wmb();

	/* if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark))
		ret = (behavior == RING_QUEUE_FIXED) ? -EDQUOT :
				(int)(n | RING_QUOT_EXCEED);
	else
		ret = (behavior == RING_QUEUE_FIXED) ? 0 : n;

	/*
	 * If there are other enqueues in progress that preceded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(atomic_read(&r->prod.tail) != prod_head)) {
		cpu_relax();

		/* Set RING_PAUSE_REP_COUNT to avoid spin too long waiting
		 * for other thread finish. It gives pre-empted thread a chance
		 * to proceed and finish with ring dequeue operation. */
		if (RING_PAUSE_REP_COUNT &&
		    ++rep == RING_PAUSE_REP_COUNT) {
			rep = 0;
			schedule();
		}
	}
	atomic_set(&r->prod.tail, prod_next);
	return ret;
}

/**
 * @internal Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @param behavior
 *   RING_QUEUE_FIXED:    Enqueue a fixed number of items from a ring
 *   RING_QUEUE_VARIABLE: Enqueue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RING_QUEUE_FIXED
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 *   if behavior = RING_QUEUE_VARIABLE
 *   - n: Actual number of objects enqueued.
 */
static inline int __attribute__((always_inline))
__ring_sp_do_enqueue(struct ring *r, void * const *obj_table,
	unsigned n, enum ring_queue_behavior behavior)
{
	uint32_t prod_head, cons_tail;
	uint32_t prod_next, free_entries;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	prod_head = atomic_read(&r->prod.head);
	cons_tail = atomic_read(&r->cons.tail);
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * prod_head > cons_tail). So 'free_entries' is always between 0
	 * and size(ring)-1. */
	free_entries = mask + cons_tail - prod_head;

	/* check that we have enough room in ring */
	if (unlikely(n > free_entries)) {
		if (behavior == RING_QUEUE_FIXED)
			return -ENOBUFS;
		else {
			/* No free entry available */
			if (unlikely(free_entries == 0))
				return 0;
			n = free_entries;
		}
	}

	prod_next = prod_head + n;
	atomic_set(&r->prod.head, prod_next);

	/* write entries in ring */
	ENQUEUE_PTRS();
	smp_wmb();

	/* if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark))
		ret = (behavior == RING_QUEUE_FIXED) ? -EDQUOT :
			(int)(n | RING_QUOT_EXCEED);
	else
		ret = (behavior == RING_QUEUE_FIXED) ? 0 : n;

	atomic_set(&r->prod.tail, prod_next);
	return ret;
}

/**
 * @internal Dequeue several objects from a ring (multi-consumers safe). When
 * the request objects are more than the available objects, only dequeue the
 * actual number of objects
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @param behavior
 *   RING_QUEUE_FIXED:    Dequeue a fixed number of items from a ring
 *   RING_QUEUE_VARIABLE: Dequeue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RING_QUEUE_FIXED
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 *   if behavior = RING_QUEUE_VARIABLE
 *   - n: Actual number of objects dequeued.
 */

static inline int __attribute__((always_inline))
__ring_mc_do_dequeue(struct ring *r, void **obj_table,
	unsigned n, enum ring_queue_behavior behavior)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	const unsigned max = n;
	int success;
	unsigned i, rep = 0;
	uint32_t mask = r->prod.mask;

	/* Avoid the unnecessary cmpset operation below, which is also
	 * potentially harmful when n equals 0. */
	if (n == 0)
		return 0;

	/* move cons.head atomically */
	do {
		/* Restore n as it may change every loop */
		n = max;

		cons_head = atomic_read(&r->cons.head);
		prod_tail = atomic_read(&r->prod.tail);
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * cons_head > prod_tail). So 'entries' is always between 0
		 * and size(ring)-1. */
		entries = (prod_tail - cons_head);

		/* Set the actual entries for dequeue */
		if (n > entries) {
			if (behavior == RING_QUEUE_FIXED)
				return -ENOENT;
			else {
				if (unlikely(entries == 0))
					return 0;
				n = entries;
			}
		}

		cons_next = cons_head + n;
		success = atomic_cmpxchg(&r->cons.head, cons_head,
					      cons_next);
	} while (unlikely(success == 0));

	/* copy in table */
	DEQUEUE_PTRS();
	smp_rmb();

	/*
	 * If there are other dequeues in progress that preceded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(atomic_read(&r->cons.tail) != cons_head)) {
		cpu_relax();

		/* Set RING_PAUSE_REP_COUNT to avoid spin too long waiting
		 * for other thread finish. It gives pre-empted thread a chance
		 * to proceed and finish with ring dequeue operation. */
		if (RING_PAUSE_REP_COUNT &&
		    ++rep == RING_PAUSE_REP_COUNT) {
			rep = 0;
			schedule();
		}
	}
	atomic_set(&r->cons.tail, cons_next);

	return behavior == RING_QUEUE_FIXED ? 0 : n;
}


/**
 * @internal Dequeue several objects from a ring (NOT multi-consumers safe).
 * When the request objects are more than the available objects, only dequeue
 * the actual number of objects
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @param behavior
 *   RING_QUEUE_FIXED:    Dequeue a fixed number of items from a ring
 *   RING_QUEUE_VARIABLE: Dequeue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RING_QUEUE_FIXED
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 *   if behavior = RING_QUEUE_VARIABLE
 *   - n: Actual number of objects dequeued.
 */
static inline int __attribute__((always_inline))
__ring_sc_do_dequeue(struct ring *r, void **obj_table,
	unsigned n, enum ring_queue_behavior behavior)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	unsigned i;
	uint32_t mask = r->prod.mask;

	cons_head = atomic_read(&r->cons.head);
	prod_tail = atomic_read(&r->prod.tail);
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * cons_head > prod_tail). So 'entries' is always between 0
	 * and size(ring)-1. */
	entries = prod_tail - cons_head;

	if (n > entries) {
		if (behavior == RING_QUEUE_FIXED)
			return -ENOENT;
		else {
			if (unlikely(entries == 0))
				return 0;
			n = entries;
		}
	}

	cons_next = cons_head + n;
	atomic_set(&r->cons.head, cons_next);

	/* copy in table */
	DEQUEUE_PTRS();
	smp_rmb();

	atomic_set(&r->cons.tail, cons_next);
	return behavior == RING_QUEUE_FIXED ? 0 : n;
}

/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 */
static inline int __attribute__((always_inline))
ring_mp_enqueue_bulk(struct ring *r, void * const *obj_table,
	unsigned n)
{
	return __ring_mp_do_enqueue(r, obj_table, n, RING_QUEUE_FIXED);
}

/**
 * Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
ring_sp_enqueue_bulk(struct ring *r, void * const *obj_table,
	unsigned n)
{
	return __ring_sp_do_enqueue(r, obj_table, n, RING_QUEUE_FIXED);
}

/**
 * Enqueue several objects on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version depending on the default behavior that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
ring_enqueue_bulk(struct ring *r, void * const *obj_table,
	unsigned n)
{
	if (r->prod.sp_enqueue)
		return ring_sp_enqueue_bulk(r, obj_table, n);
	else
		return ring_mp_enqueue_bulk(r, obj_table, n);
}

/**
 * Enqueue one object on a ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
ring_mp_enqueue(struct ring *r, void *obj)
{
	return ring_mp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
ring_sp_enqueue(struct ring *r, void *obj)
{
	return ring_sp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int __attribute__((always_inline))
ring_enqueue(struct ring *r, void *obj)
{
	if (r->prod.sp_enqueue)
		return ring_sp_enqueue(r, obj);
	else
		return ring_mp_enqueue(r, obj);
}

/**
 * Dequeue several objects from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
ring_mc_dequeue_bulk(struct ring *r, void **obj_table, unsigned n)
{
	return __ring_mc_do_dequeue(r, obj_table, n, RING_QUEUE_FIXED);
}

/**
 * Dequeue several objects from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table,
 *   must be strictly positive.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
ring_sc_dequeue_bulk(struct ring *r, void **obj_table, unsigned n)
{
	return __ring_sc_do_dequeue(r, obj_table, n, RING_QUEUE_FIXED);
}

/**
 * Dequeue several objects from a ring.
 *
 * This function calls the multi-consumers or the single-consumer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
ring_dequeue_bulk(struct ring *r, void **obj_table, unsigned n)
{
	if (r->cons.sc_dequeue)
		return ring_sc_dequeue_bulk(r, obj_table, n);
	else
		return ring_mc_dequeue_bulk(r, obj_table, n);
}

/**
 * Dequeue one object from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
ring_mc_dequeue(struct ring *r, void **obj_p)
{
	return ring_mc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
ring_sc_dequeue(struct ring *r, void **obj_p)
{
	return ring_sc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring.
 *
 * This function calls the multi-consumers or the single-consumer
 * version depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success, objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static inline int __attribute__((always_inline))
ring_dequeue(struct ring *r, void **obj_p)
{
	if (r->cons.sc_dequeue)
		return ring_sc_dequeue(r, obj_p);
	else
		return ring_mc_dequeue(r, obj_p);
}

/**
 * Test if a ring is full.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is full.
 *   - 0: The ring is not full.
 */
static inline int
ring_full(const struct ring *r)
{
	uint32_t prod_tail = atomic_read(&r->prod.tail);
	uint32_t cons_tail = atomic_read(&r->cons.tail);
	return ((cons_tail - prod_tail - 1) & r->prod.mask) == 0;
}

/**
 * Test if a ring is empty.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is empty.
 *   - 0: The ring is not empty.
 */
static inline int
ring_empty(const struct ring *r)
{
	uint32_t prod_tail = atomic_read(&r->prod.tail);
	uint32_t cons_tail = atomic_read(&r->cons.tail);
	return !!(cons_tail == prod_tail);
}

/**
 * Return the number of entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of entries in the ring.
 */
static inline unsigned
ring_count(const struct ring *r)
{
	uint32_t prod_tail = atomic_read(&r->prod.tail);
	uint32_t cons_tail = atomic_read(&r->cons.tail);
	return (prod_tail - cons_tail) & r->prod.mask;
}

/**
 * Return the number of free entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of free entries in the ring.
 */
static inline unsigned
ring_free_count(const struct ring *r)
{
	uint32_t prod_tail = atomic_read(&r->prod.tail);
	uint32_t cons_tail = atomic_read(&r->cons.tail);
	return (cons_tail - prod_tail - 1) & r->prod.mask;
}

/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - n: Actual number of objects enqueued.
 */
static inline unsigned __attribute__((always_inline))
ring_mp_enqueue_burst(struct ring *r, void * const *obj_table,
	unsigned n)
{
	return __ring_mp_do_enqueue(r, obj_table, n, RING_QUEUE_VARIABLE);
}

/**
 * Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - n: Actual number of objects enqueued.
 */
static inline unsigned __attribute__((always_inline))
ring_sp_enqueue_burst(struct ring *r, void * const *obj_table,
			 unsigned n)
{
	return __ring_sp_do_enqueue(r, obj_table, n, RING_QUEUE_VARIABLE);
}

/**
 * Enqueue several objects on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version depending on the default behavior that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - n: Actual number of objects enqueued.
 */
static inline unsigned __attribute__((always_inline))
ring_enqueue_burst(struct ring *r, void * const *obj_table,
		      unsigned n)
{
	if (r->prod.sp_enqueue)
		return ring_sp_enqueue_burst(r, obj_table, n);
	else
		return ring_mp_enqueue_burst(r, obj_table, n);
}

/**
 * Dequeue several objects from a ring (multi-consumers safe). When the request
 * objects are more than the available objects, only dequeue the actual number
 * of objects
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - n: Actual number of objects dequeued, 0 if ring is empty
 */
static inline unsigned __attribute__((always_inline))
ring_mc_dequeue_burst(struct ring *r, void **obj_table, unsigned n)
{
	return __ring_mc_do_dequeue(r, obj_table, n, RING_QUEUE_VARIABLE);
}

/**
 * Dequeue several objects from a ring (NOT multi-consumers safe).When the
 * request objects are more than the available objects, only dequeue the
 * actual number of objects
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - n: Actual number of objects dequeued, 0 if ring is empty
 */
static inline unsigned __attribute__((always_inline))
ring_sc_dequeue_burst(struct ring *r, void **obj_table, unsigned n)
{
	return __ring_sc_do_dequeue(r, obj_table, n, RING_QUEUE_VARIABLE);
}

/**
 * Dequeue multiple objects from a ring up to a maximum number.
 *
 * This function calls the multi-consumers or the single-consumer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - Number of objects dequeued
 */
static inline unsigned __attribute__((always_inline))
ring_dequeue_burst(struct ring *r, void **obj_table, unsigned n)
{
	if (r->cons.sc_dequeue)
		return ring_sc_dequeue_burst(r, obj_table, n);
	else
		return ring_mc_dequeue_burst(r, obj_table, n);
}
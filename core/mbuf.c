/*
 * mbuf.c - buffer management for network packets
 *
 * TODO: add support for mapping into user-level address space...
 */

#include <kns/mem.h>
#include <kns/mempool.h>
#include <kns/mbuf.h>

/* Capacity should be at least RX queues per CPU * ETH_DEV_RX_QUEUE_SZ */
#define MBUF_CAPACITY	(768*1024)

struct mempool_datastore mbuf_datastore;

DEFINE_PER_CPU(struct mempool, mbuf_mempool __attribute__ ((aligned (64))));

void mbuf_default_done(struct mbuf *m)
{
	mbuf_free(m);
}

/**
 * mbuf_init_cpu - allocates the core-local mbuf region
 *
 * Returns 0 if successful, otherwise failure.
 */

int mbuf_init_cpu(void)
{
	struct mempool *m = this_cpu_ptr(&mbuf_mempool);
	return mempool_create(m, &mbuf_datastore, MEMPOOL_SANITY_PERCPU,smp_processor_id());
}

/**
 * mbuf_init - allocate global mbuf
 */

int mbuf_init(void)
{
	int ret;
	struct mempool_datastore *m = &mbuf_datastore;
	BUG_ON(sizeof(struct mbuf) <= MBUF_HEADER_LEN);

	ret = mempool_create_datastore(m, MBUF_CAPACITY, MBUF_LEN,1,MEMPOOL_DEFAULT_CHUNKSIZE,"mbuf");
	if (ret) {
		BUG_ON(0);
		return ret;
	}
	ret = mempool_pagemem_map_to_user(m);
	if (ret) {
		BUG_ON(0);
		mempool_pagemem_destroy(m);
		return ret;
	}
	return 0;
}

/**
 * mbuf_exit_cpu - frees the core-local mbuf region
 */
void mbuf_exit_cpu(void)
{
	mempool_pagemem_destroy(&mbuf_datastore);
}

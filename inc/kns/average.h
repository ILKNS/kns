/*
 * Adapted from linux/average.h
 */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_AVERAGE_H
#define _LINUX_AVERAGE_H
#include <linux/bug.h>
#include <linux/compiler.h>
#include <linux/log2.h>
/*
 * Exponentially weighted moving average (EWMA)
 *
 * This implements a fixed-precision EWMA algorithm, with both the
 * precision and fall-off coefficient determined at compile-time
 * and built into the generated helper funtions.
 *
 * The first argument to the macro is the name that will be used
 * for the struct and helper functions.
 *
 * The second argument, the precision, expresses how many bits are
 * used for the fractional part of the fixed-precision values.
 *
 * The third argument, the weight reciprocal, determines how the
 * new values will be weighed vs. the old state, new values will
 * get weight 1/weight_rcp and old values 1-1/weight_rcp. Note
 * that this parameter must be a power of two for efficiency.
 */
struct ewma{
	unsigned long internal;
	int precision;
	int weight_rcp;
};
	
static inline void ewma_init(volatile struct ewma *e, int _precision, int _weight_rcp)
{
	BUILD_BUG_ON(!__builtin_constant_p(_precision));
	BUILD_BUG_ON(!__builtin_constant_p(_weight_rcp));
	/*
	 * Even if you want to feed it just 0/1 you should have
	 * some bits for the non-fractional part...
	 */
	BUILD_BUG_ON((_precision) > 30);
	BUILD_BUG_ON_NOT_POWER_OF_2(_weight_rcp);

	e->internal = 0;
	e->precision = _precision;
	e->weight_rcp = _weight_rcp;
}

static inline unsigned long	ewma_read(volatile struct ewma *e)
{
	return e->internal >> (e->precision);
}

static inline void ewma_add(volatile struct ewma *e,
	unsigned long val)
{
	unsigned long internal = READ_ONCE(e->internal);
	unsigned long weight_rcp = ilog2(e->weight_rcp);
	unsigned long precision = e->precision;

	WRITE_ONCE(e->internal, internal ?
		(((internal << weight_rcp) - internal) +
			(val << precision)) >> weight_rcp :
			(val << precision));
}

#endif /* _LINUX_AVERAGE_H */
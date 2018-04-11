/* a wrapper of hrtimer api in linux/hrtimer.h */

#pragma once

#include <linux/hrtimer.h>

#define ONE_SECOND	1000000
#define ONE_MS		1000
#define ONE_US		1

struct eth_fg;

struct kns_hrtimer{
	struct hrtimer hrt;
	int fg_id;
};
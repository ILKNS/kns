/*
 * Copyright 2013-16 Board of Trustees of Stanford University
 * Copyright 2013-16 Ecole Polytechnique Federale Lausanne (EPFL)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * control_plane.h - control plane definitions
 */

#include <kns/ethfg.h>
#include <kns/average.h>

#define IDLE_FIFO_SIZE 256

#define EMA_SMOOTH_FACTOR_0 2
#define EMA_SMOOTH_FACTOR_1 4
#define EMA_SMOOTH_FACTOR_2 8
#define EMA_SMOOTH_FACTOR EMA_SMOOTH_FACTOR_0

struct cpu_metrics {
	struct ewma queuing_delay;
	struct ewma batch_size;
	struct ewma queue_size[3];
	struct ewma loop_duration;
	struct ewma idle[3];
} __aligned(64);

struct flow_group_metrics {
	int cpu;
} __aligned(64);

enum cpu_state {
	CP_CPU_STATE_IDLE = 0,
	CP_CPU_STATE_RUNNING,
};

enum commands {
	CP_CMD_NOP = 0,
	CP_CMD_MIGRATE,
	CP_CMD_IDLE,
};

enum status {
	CP_STATUS_READY = 0,
	CP_STATUS_RUNNING,
};

struct command_struct {
	enum cpu_state cpu_state;
	enum commands cmd_id;
	enum status status;
	union {
		struct {
			DECLARE_BITMAP(fg_bitmap, ETH_MAX_TOTAL_FG);
			int cpu;
		} migrate;
		struct {
			char fifo[IDLE_FIFO_SIZE];
		} idle;
	};
	char no_idle;
};

extern volatile struct cp_shmem {
	uint32_t nr_flow_groups;
	uint32_t nr_cpus;
	unsigned long pkg_power;
	int cpu[NR_CPUS];
	struct cpu_metrics cpu_metrics[NR_CPUS];
	struct flow_group_metrics flow_group[ETH_MAX_TOTAL_FG];
	struct command_struct command[NR_CPUS];
	uint32_t cycles_per_us;
	uint32_t scratchpad_idx;
	struct {
		long remote_queue_pkts_begin;
		long remote_queue_pkts_end;
		long local_queue_pkts;
		long backlog_before;
		long backlog_after;
		long timers;
		long timer_fired;
		long ts_migration_start;
		long ts_data_structures_done;
		long ts_before_backlog;
		long ts_after_backlog;
		long ts_migration_end;
		long ts_first_pkt_at_prev;
		long ts_last_pkt_at_prev;
		long ts_first_pkt_at_target;
		long ts_last_pkt_at_target;
	} scratchpad[1024];
} *cp_shmem;

#define SCRATCHPAD (&cp_shmem->scratchpad[cp_shmem->scratchpad_idx])
#define SCRATCHPAD_NEXT do { BUG_ON(++cp_shmem->scratchpad_idx < 1024); } while (0)

/*FIXME: removed volatile, might be wrong*/
DECLARE_PER_CPU(struct command_struct *, cp_cmd);
DECLARE_PER_CPU(unsigned long, idle_cycles);

void cp_idle(void);

extern unsigned long energy_unit;
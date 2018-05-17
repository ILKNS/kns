# Highres Timer In Linux
Linux provide timer with high resolution, and I read several codes of Linux driver find that the two main factor to adapt it
- The callback function: to be called after the time up.
- Hold the hrtimer in the structure bound with it which is easy.
- [highres timer API intro](https://lwn.net/Articles/167897/)

# ktime_t
- `ktime_set` : Set a ktime_t variable from a seconds/nanoseconds value
- `ktime_sub`, `ktime_add`, `ktime_add_ns`, `ktime_sub_ns`  
- `ktime_to_ns`  
- `ktime_equal`
- `hrtimer_cb_get_time`

# hrtimer
- Have some trouble with `hrtimer_state`
- `hrtimer`: 
    - `_softexpires`, `function`, `base` and `state` are frequently used.
- API
    - `hrtimer_init`
    - `hrtimer_set_expires`
    - `hrtimer_add_expires`
    - `hrtimer_cb_get_time`
    - `hrtimer_active`
    - `hrtimer_get_remaining`
    - `hrtimer_start`&`hrtimer_start_expires`
    - `hrtimer_restart`&`hrtimer_cancel`
    

# Timer in IX
We provide a hierarchical timing wheel implementation for managing network timeouts, such as TCP retransmissions. It is optimized for the common case where most timers are canceled before they expire.  We support extremely high-resolution timeouts, as low as 16μs, which has been shown to improve performance during TCP incast congestion.

A timer event infrastructure
```
struct timer {
	struct hlist_node link;
	void (*handler)(struct timer *t, struct eth_fg *cur_fg);
	uint64_t expires;
	int fg_id;
};

struct timerwheel {
	uint64_t now_us;
	uint64_t timer_pos;
	struct hlist_head wheels[WHEEL_COUNT][WHEEL_SIZE];

};
```
In which timerwheel is declared per cpu variable.
And in delay procedure `rdtsc()` and `cpu_relax` are used to get cpu cycles and wait.

I checked the reimplement of timer in IX, and only some functions need to be port to kernel as a wrapper of some functions.

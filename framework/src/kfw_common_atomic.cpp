
#include "kfw_common_atomic.hpp"

namespace kfw
{

	int32_t atomic_inc(volatile int32_t &value) {
		AtomicLock lock();
		return ++value;
	}

	int32_t atomic_dec(volatile int32_t &value) {
		AtomicLock lock();
		return --value;
	}

	uint32_t atomic_inc(volatile uint32_t &value) {
		AtomicLock lock();
		return ++value;
	}

	uint32_t atomic_dec(volatile uint32_t &value) {
		AtomicLock lock();
		return --value;
	}

	uint32_t atomic_inc(volatile uint32_t *value) {
		AtomicLock lock();
		return ++*value;
	}

	uint32_t atomic_dec(volatile uint32_t *value) {
		AtomicLock lock();
		return --*value;
	}

	void atomic_lock(interlock_stat_t &lock_stat) {
		lock_stat = sns_loc();
		loc_cpu();
	}
	void atomic_unlock(interlock_stat_t lock_stat)
	{
		if(lock_stat) {
			unl_cpu();
		}
	}

}

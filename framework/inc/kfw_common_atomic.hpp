#pragma once

#include <itron.h>
#include <kfw_common_base.hpp>

namespace kfw
{
	using interlock_stat_t = BOOL;

	void atomic_lock(interlock_stat_t &lock_stat);
	void atomic_unlock(interlock_stat_t lock_stat);

	class AtomicLock : private NonCopyable
	{
	public:
		AtomicLock() {
			atomic_lock(m_stat);
		}

		~AtomicLock() {
			atomic_unlock(m_stat);
		}

	private:
		interlock_stat_t m_stat;
	};

	int32_t atomic_inc(volatile int32_t &value);
	int32_t atomic_dec(volatile int32_t &value);
	uint32_t atomic_inc(volatile uint32_t &value);
	uint32_t atomic_dec(volatile uint32_t &value);
	uint32_t atomic_inc(volatile uint32_t *value);
	uint32_t atomic_dec(volatile uint32_t *value);
};

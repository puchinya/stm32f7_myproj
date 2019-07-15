
#pragma once
#ifndef KFW_RTOS_HPP
#define KFW_RTOS_HPP

#include <kfw_common.hpp>
#include <kfw_callback.hpp>
#include <itron.h>

#ifndef CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
#define CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR	1
#endif

namespace kfw { namespace rtos {

	using thread_handle_t = ID;
	using semaphore_handle_t = ID;
	using mutex_handle_t = ID;
	using eventflag_handle_t = ID;
	using priority_t = PRI;

	constexpr thread_handle_t kInvalidThreadHandle = TSK_NONE;
	constexpr semaphore_handle_t kInvalidSemaphoreHandle = 0;
	constexpr mutex_handle_t kInvalidMutexHandle = 0;
	constexpr eventflag_handle_t kInvalidEventFlagHandle = 0;

	using timeout_t = int32_t;
	constexpr timeout_t kTimeoutInfinity = -1;
	constexpr timeout_t kTimeoutNoWait = 0;

	constexpr priority_t kPriorityHigh = 1;
	constexpr priority_t kPriorityNormal = 2;
	constexpr priority_t kPriorityLow = 3;

	/**
	 *
	 */
	class Semaphore final : private NonCopyable
	{
	public:
		Semaphore() : m_handle(kInvalidSemaphoreHandle) {}
		Semaphore(Semaphore &&obj) {
			m_handle = obj.m_handle;
			obj.m_handle = kInvalidSemaphoreHandle;
		}

		~Semaphore() { dispose(); }

		ret_t create(uint32_t count, uint32_t max_count);

		void dispose();

		ret_t wait(timeout_t timeout_ms = kTimeoutInfinity);
		ret_t release();
		ret_t release_isr();

		Semaphore &operator = (Semaphore &&obj)
		{
			dispose();
			this->m_handle = obj.m_handle;
			obj.m_handle = kInvalidSemaphoreHandle;
			return *this;
		}
	private:
		semaphore_handle_t m_handle;
	};

	/**
	 *
	 */
	class Mutex final : private NonCopyable
	{
	public:
		Mutex() : m_handle(kInvalidMutexHandle) {}
		~Mutex() {
			dispose();
		}

		ret_t create();
		void dispose();

		ret_t lock();
		ret_t unlock();
		ret_t unlock_isr();
	private:
		mutex_handle_t m_handle;
	};

	/**
	 *
	 */
	enum class EventFlagsWaitMode : MODE
	{
		kAnd = TWF_ANDW,
		kOr = TWF_ORW
	};

	/**
	 *
	 */
	class EventFlags final : private NonCopyable
	{
	public:
		EventFlags() : m_handle(kInvalidEventFlagHandle) {}
		~EventFlags() {
			dispose();
		}

		ret_t create(uint32_t pattern = 0);
		void dispose();

		ret_t set(uint32_t pattern);
		ret_t clear(uint32_t pattern = 0);
		ret_t wait(uint32_t pattern, EventFlagsWaitMode mode,
				uint32_t &flag_pattern,
				timeout_t timeout_ms = kTimeoutInfinity);
		ret_t wait(uint32_t pattern, EventFlagsWaitMode mode,
				timeout_t timeout_ms = kTimeoutInfinity)
		{
			uint32_t flag_pattern;
			return wait(pattern, mode, flag_pattern, timeout_ms);
		}
		ret_t ser_isr(uint32_t pattern);
	private:
		eventflag_handle_t m_handle;
	};

	class MemoryPool final : private NonCopyable
	{
	public:
		MemoryPool();
		~MemoryPool();

		ret_t create(uint32_t block_size, uint32_t block_count);
		ret_t dispose();

		ret_t get(void **block, timeout_t timeout_ms = kTimeoutInfinity);
		ret_t release(void *block);

		uint32_t get_block_size() const;
	};

	class DataQueue final : private NonCopyable
	{
	public:
		ret_t create(uint32_t ququ_count);
		ret_t dispose();

		ret_t send(uintptr_t data, timeout_t timeout_ms = kTimeoutInfinity);
		ret_t recv(uintptr_t &data, timeout_t timeout_ms = kTimeoutInfinity);
	private:
		ID m_handle;
	};

	class ThreadContext final : private NonCopyable
	{
		friend class Thread;
	public:
		~ThreadContext() {
			dispose();
		}
	private:
		ThreadContext() : m_handle(kInvalidThreadHandle) {}

		thread_handle_t m_handle;
		Callback<void()> m_func;
		EventFlags m_join_flag;

		static void entry_point(kos_vp_t p);

		ret_t create(const Callback<void()> &func, priority_t priority,
				uint32_t stack_size, void *stack_mem = nullptr);

		void dispose();
	};

	/**
	 *	@brief スレッドクラス
	 */
	class Thread final : private NonCopyable
	{
	private:
		ThreadContext *m_context;
	public:
		Thread() : m_context(nullptr) {}
		~Thread() {
			dispose();
		}

		ret_t create(const Callback<void()> &func, priority_t priority,
				uint32_t stack_size, void *stack_mem = nullptr);

		void dispose();

		ret_t start();

		ret_t join();

		ret_t set_priority(priority_t value);

		ret_t get_priority(priority_t &value);

		static void sleep(uint32_t ms);
	};

};};

#endif

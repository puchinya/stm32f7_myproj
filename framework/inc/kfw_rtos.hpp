
#pragma once
#ifndef KFW_RTOS_HPP
#define KFW_RTOS_HPP

#include <kfw_common.hpp>
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
	enum class EventFlagWaitMode : MODE
	{
		kAnd = TWF_ANDW,
		kOr = TWF_ORW
	};

	/**
	 *
	 */
	class EventFlag final : private NonCopyable
	{
	public:
		EventFlag() : m_handle(kInvalidEventFlagHandle) {}
		~EventFlag() {
			dispose();
		}

		ret_t create(uint32_t pattern = 0);
		void dispose();

		/**
		 * @brief フラグパターンを設定します。(スレッドコンテキスト用)
		 */
		ret_t set(uint32_t pattern);

		/**
		 * @brief フラグパターンをクリアします。(スレッドコンテキスト用)
		 */
		ret_t clear(uint32_t pattern = 0);
		ret_t wait(uint32_t pattern, EventFlagWaitMode mode,
				uint32_t &flag_pattern,
				timeout_t timeout_ms = kTimeoutInfinity);
		ret_t wait(uint32_t pattern, EventFlagWaitMode mode,
				timeout_t timeout_ms = kTimeoutInfinity)
		{
			uint32_t flag_pattern;
			return wait(pattern, mode, flag_pattern, timeout_ms);
		}

		/**
		 * @brief フラグパターンを設定します。(割り込みサービスルーチン用)
		 */
		ret_t set_isr(uint32_t pattern);
	private:
		eventflag_handle_t m_handle;
	};

	struct FixedMemoryPoolBlock
	{
		FixedMemoryPoolBlock	*m_next;
		uint8_t					m_data[1];
	};

	class FixedMemoryPool final : private NonCopyable
	{
	public:
		FixedMemoryPool()
		: m_buffer(nullptr), m_allocated_buffer_size(0),
		  m_buffer_size(0), m_block_size(0), m_free_list(nullptr)
		{

		}

		~FixedMemoryPool() {
			dispose();
		}

		ret_t create(uint32_t block_size, uint32_t block_count);
		void dispose();

		ret_t get(void **block, timeout_t timeout_ms = kTimeoutInfinity);
		ret_t get_isr(void **block);
		ret_t release(void *block);

		uint32_t get_block_size() const {
			return m_block_size;
		}
	private:
		uint8_t *m_buffer;
		uint32_t m_allocated_buffer_size;
		uint32_t m_buffer_size;
		uint32_t m_block_size;
		FixedMemoryPoolBlock *m_free_list;
		Semaphore m_sem;
	};

	class DataQueue final : private NonCopyable
	{
	public:
		DataQueue()
		: m_handle(0) {}
		~DataQueue() {
			dispose();
		}

		/**
		 * @brief データキューオブジェクトの初期化を行います。
		 * @param[in]	queue_count	キューの長さ
		 * @retval	kOk					成功
		 * @retval	kEInvalidOperation	すでに初期化済み
		 * @retval	kEResouece			OSリソースの確保に失敗。
		 */
		ret_t create(uint32_t queue_count);

		/**
		 * @brief オブジェクトにて解放しているリソースを破棄します。
		 */
		void dispose();

		/**
		 *	@brief	データキューにデータを入れます。
		 *	@param[in]	data		キューに入れるデータ
		 *	@param[in]	timeout_ms	キューに空きが出るまでのタイムアウト時間[ms]
		 *	@retval		kOk			成功
		 *	@retval		0未満		エラー
		 *	キューに空きがない時にはキューに空きができるまで、待機します。
		 */
		ret_t send(uintptr_t data, timeout_t timeout_ms = kTimeoutInfinity);
		ret_t recv(uintptr_t &data, timeout_t timeout_ms = kTimeoutInfinity);
		ret_t send_isr(uintptr_t data);
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
		EventFlag m_join_flag;

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

	// @TODO
	class Timer final : private NonCopyable
	{
	public:
		Timer() : m_handle(0) {

		}
		~Timer() {
			dispose();
		}

		ret_t create(const Callback<void()> &func,
				uint32_t interval);
		void dispose();
		ret_t start();
		void stop();
	private:
		Callback<void()> m_func;
		ID m_handle;
		static void handler_entry(Timer *obj);
	};

	void kfw_rtos_static_init();
};};

#endif

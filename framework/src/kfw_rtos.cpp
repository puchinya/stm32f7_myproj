
#include "kfw_rtos.hpp"
#include "kfw_system.hpp"

namespace kfw { namespace rtos {

static ret_t kfw_convert_er(ER er)
{
	switch(er) {
	case E_OK: return kOk;
	case E_TMOUT: return kETimeout;
	default:
		return kEUnknown;
	}
}

static TMO convert_timeout(timeout_t tm)
{
	switch(tm) {
	case kTimeoutNoWait:
		return TMO_POL;
	case kTimeoutInfinity:
		return TMO_FEVR;
	default:
		return tm * KOS_TIC_NUME / KOS_TIC_DENO;
	}
}

ret_t Semaphore::create(uint32_t count, uint32_t max_count)
{
	if(m_handle != kInvalidSemaphoreHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEInvalidOperation);
#endif
		return kEInvalidOperation;
	}

	kos_csem_t csem = {0};
	csem.isemcnt = count;
	csem.maxsem = max_count;

	kos_er_id_t r = kos_cre_sem(&csem);
	if(r < 0) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEResource);
#endif
		return kEResource;
	}
	m_handle = r;

	return kOk;
}

void Semaphore::dispose()
{
	if(m_handle == kInvalidSemaphoreHandle) {
		return;;
	}

	del_sem(m_handle);
	m_handle = kInvalidSemaphoreHandle;
}

ret_t Semaphore::wait(timeout_t timeout_ms)
{
	if(m_handle == kInvalidSemaphoreHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEDisposed);
#endif
		return kEDisposed;
	}

	kos_er_t r = twai_sem(m_handle, timeout_ms);

	return kfw_convert_er(r);
}

ret_t Semaphore::release()
{
	if(m_handle == kInvalidSemaphoreHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEDisposed);
#endif
		return kEDisposed;
	}

	kos_er_t r = sig_sem(m_handle);

	return kfw_convert_er(r);
}

ret_t Semaphore::release_isr()
{
	if(m_handle == kInvalidSemaphoreHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEDisposed);
#endif
		return kEDisposed;
	}

	kos_er_t r = isig_sem(m_handle);

	return kfw_convert_er(r);
}

ret_t Mutex::create()
{
	if(m_handle != kInvalidMutexHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEInvalidOperation);
#endif
		return kEInvalidOperation;
	}

	kos_cmtx_t cmtx = {};

	kos_er_id_t r = kos_cre_mtx(&cmtx);

	if(r < 0) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEResource);
#endif
		return kEResource;
	}

	m_handle = r;

	return kOk;
}

void Mutex::dispose()
{
	if(m_handle == kInvalidMutexHandle) {
		return;
	}

	kos_del_mtx(m_handle);
	m_handle = kInvalidMutexHandle;
}

ret_t Mutex::lock()
{
	if(m_handle == kInvalidMutexHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEDisposed);
#endif
		return kEDisposed;
	}

	kos_er_t er = kos_loc_mtx(m_handle);

	return kfw_convert_er(er);
}

ret_t Mutex::unlock()
{
	if(m_handle == kInvalidMutexHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEDisposed);
#endif
		return kEDisposed;
	}

	kos_er_t er = kos_unl_mtx(m_handle);

	return kfw_convert_er(er);
}

ret_t Mutex::unlock_isr()
{
	if(m_handle == kInvalidMutexHandle) {
#if CFG_KFW_RTOS_RAISE_SYSTEM_ERROR_WHEN_FATAL_ERROR
		kfw_system_error(kEDisposed);
#endif
		return kEDisposed;
	}

	kos_er_t er = kos_unl_mtx(m_handle);

	return kfw_convert_er(er);
}

ret_t EventFlag::create(uint32_t pattern)
{
	if(m_handle != kInvalidEventFlagHandle) {
		return kEInvalidOperation;
	}

	T_CFLG cflg = {0};

	cflg.flgatr = TA_HLNG;
	cflg.iflgptn = pattern;

	ER_ID er_id = acre_flg(&cflg);
	if(er_id < 0) {
		return kEResource;
	}

	m_handle = er_id;

	return kOk;
}

void EventFlag::dispose()
{
	if(m_handle == kInvalidEventFlagHandle) {
		return;
	}

	del_flg(m_handle);
	m_handle = kInvalidEventFlagHandle;
}

ret_t EventFlag::set(uint32_t flags)
{
	if(m_handle == kInvalidEventFlagHandle) {
		return kEDisposed;
	}

	ER er = set_flg(m_handle, flags);

	return kfw_convert_er(er);
}

ret_t EventFlag::clear(uint32_t pattern)
{
	if(m_handle == kInvalidEventFlagHandle) {
		return kEDisposed;
	}

	ER er = clr_flg(m_handle, pattern);

	return kfw_convert_er(er);
}

ret_t EventFlag::wait(uint32_t pattern, EventFlagWaitMode mode,
		uint32_t &flag_pattern,
		timeout_t timeout_ms)
{
	if(m_handle == kInvalidEventFlagHandle) {
		return kEDisposed;
	}

	TMO tmo = convert_timeout(timeout_ms);
	FLGPTN flgptn;

	ER er = twai_flg(m_handle, pattern, (MODE)mode, &flgptn, tmo);

	return kfw_convert_er(er);
}

ret_t EventFlag::set_isr(uint32_t pattern)
{
	if(m_handle == kInvalidEventFlagHandle) {
		return kEDisposed;
	}

	ER er = iset_flg(m_handle, pattern);

	return kfw_convert_er(er);
}

//--- FixedMemoryPool class ---
ret_t FixedMemoryPool::create(uint32_t block_size, uint32_t block_count)
{
	if(m_buffer != nullptr) {
		return kEInvalidOperation;
	}

	uint32_t buffer_size = block_size * block_count;
	m_buffer = new uint8_t[buffer_size];
	m_allocated_buffer_size = 0;
	m_buffer_size = buffer_size;
	m_block_size = block_size;

	m_sem.create(block_count, 0xFFFFFFFF);

	return kOk;
}

void FixedMemoryPool::dispose()
{
	if(m_buffer == nullptr) {
		return;
	}

	m_sem.dispose();

	delete m_buffer;
	m_buffer = nullptr;
}

ret_t FixedMemoryPool::get(void **block, timeout_t timeout_ms)
{
	if(m_buffer == nullptr) {
		return kEInvalidOperation;
	}

	ret_t r;
	void *p;

	r = m_sem.wait(timeout_ms);
	if(is_failed(r)) {
		return r;
	}

	loc_cpu();

	if(m_allocated_buffer_size != m_buffer_size) {
		p = &m_buffer[m_allocated_buffer_size];
		m_allocated_buffer_size += m_block_size;
	} else {
		p = m_free_list;
		m_free_list = m_free_list->m_next;
	}

	unl_cpu();

	*block = p;

	return kOk;
}

ret_t FixedMemoryPool::get_isr(void **block)
{
	if(m_buffer == nullptr) {
		return kEInvalidOperation;
	}

	ret_t r;
	void *p;

	r = m_sem.release_isr();
	if(is_failed(r)) {
		return r;
	}

	loc_cpu();

	if(m_allocated_buffer_size != m_buffer_size) {
		p = &m_buffer[m_allocated_buffer_size];
		m_allocated_buffer_size += m_block_size;
	} else {
		p = m_free_list;
		m_free_list = m_free_list->m_next;
	}

	unl_cpu();

	*block = p;

	return kOk;
}
ret_t FixedMemoryPool::release(void *block)
{
	if(m_buffer == nullptr) {
		return kEInvalidOperation;
	}

	ret_t r = m_sem.release();
	if(is_failed(r)) {
		return r;
	}

	loc_cpu();

	FixedMemoryPoolBlock *b = (FixedMemoryPoolBlock *)block;

	b->m_next = m_free_list;
	m_free_list = b;

	unl_cpu();

	return kOk;
}

//--- DataQueue class ---

ret_t DataQueue::create(uint32_t queue_count)
{
	if(m_handle != 0) {
		return kEInvalidOperation;
	}

	T_CDTQ cdtq = {0};

	cdtq.dtq = nullptr;
	cdtq.dtqatr = TA_TFIFO;
	cdtq.dtqcnt = queue_count;

	ER_ID er_id = acre_dtq(&cdtq);
	if(er_id <= 0) {
		return kEResource;
	}

	m_handle = (ID)er_id;

	return kOk;
}

void DataQueue::dispose()
{
	if(m_handle == 0) {
		return;
	}

	del_dtq(m_handle);
	m_handle = 0;
}

ret_t DataQueue::send(uintptr_t data, timeout_t timeout_ms)
{
	if(m_handle == 0) {
		return kEDisposed;
	}

	ER er = tsnd_dtq(m_handle, (VP_INT)data, convert_timeout(timeout_ms));
	if(er != E_OK) {
		return kfw_convert_er(er);
	}

	return kOk;
}

ret_t DataQueue::recv(uintptr_t &data, timeout_t timeout_ms)
{
	if(m_handle == 0) {
		return kEDisposed;
	}

	ER er = trcv_dtq(m_handle, (VP_INT *)&data, convert_timeout(timeout_ms));
	if(er != E_OK) {
		return kfw_convert_er(er);
	}

	return kOk;
}

ret_t DataQueue::send_isr(uintptr_t data)
{
	if(m_handle == 0) {
		return kEDisposed;
	}

	ER er = ipsnd_dtq(m_handle, (VP_INT)data);
	if(er != E_OK) {
		return kfw_convert_er(er);
	}

	return kOk;
}

//--- ThreadContext class ---

template<typename T> inline T align_down(T value, T align)
{
	return value - (value % align);
}

template<typename T> inline T align_up(T value, T align)
{
	return value % align ? value + (align - value % align) : value;
}

void ThreadContext::entry_point(VP_INT p)
{
	ThreadContext *t = (ThreadContext *)p;

	t->m_func();
	t->m_join_flag.set(1);
}

ret_t ThreadContext::create(const Callback<void()> &func, priority_t priority,
		uint32_t stack_size, void *stack_mem)
{
	if(m_handle != kInvalidThreadHandle) {
		return kEInvalidOperation;
	}

	uintptr_t unaligned_mem = (uintptr_t)stack_mem;
	uintptr_t aligned_mem = align_up<uintptr_t>(unaligned_mem, 8);
	uint32_t offset = (uint32_t)(aligned_mem - unaligned_mem);
	uint32_t aligned_size = align_down<uint32_t>(stack_size - offset, 8);

	m_func = func;
	m_join_flag.create();

	T_CTSK ctsk = {0};

	ctsk.tskatr = TA_HLNG;
	ctsk.itskpri = priority;
	ctsk.stk = (VP)aligned_mem;
	ctsk.stksz = aligned_size;
	ctsk.exinf = (VP_INT)this;
	ctsk.task = (FP)&ThreadContext::entry_point;

	ER_ID er_id = acre_tsk(&ctsk);

	if(er_id < 0) {
		m_join_flag.dispose();
		return kfw_convert_er(er_id);
	}

	m_handle = er_id;

	return kOk;
}

void ThreadContext::dispose()
{
	if(m_handle == kInvalidThreadHandle) {
		return;
	}

	ter_tsk(m_handle);

	del_tsk(m_handle);

	m_handle = kInvalidThreadHandle;

	m_join_flag.dispose();
}

ret_t Thread::create(const Callback<void()> &func, priority_t priority,
		uint32_t stack_size, void *stack_mem)
{
	if(m_context != nullptr) {
		return kEInvalidOperation;
	}

	ThreadContext *context = new ThreadContext();
	if(context == nullptr) {
		return kEOutOfMemory;
	}

	ret_t r = context->create(func, priority, stack_size, stack_mem);

	if(is_failed(r)) {
		delete context;
		return r;
	}

	m_context = context;

	return kOk;
}

void Thread::dispose()
{
	if(m_context == nullptr) {
		return;
	}

	delete m_context;
	m_context = nullptr;
}

ret_t Thread::start()
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	ER er = sta_tsk(m_context->m_handle, m_context);

	ret_t r = kfw_convert_er(er);

	return r;

}

ret_t Thread::join() {
	if(m_context == nullptr) {
		return kEDisposed;
	}

	ret_t r = m_context->m_join_flag.wait(1, EventFlagWaitMode::kAnd);

	return r;
}

ret_t Thread::set_priority(priority_t value) {

	if(m_context == kInvalidThreadHandle) {
		return kEDisposed;
	}

	ER er = chg_pri(m_context->m_handle, value);

	return kfw_convert_er(er);
}

ret_t Thread::get_priority(priority_t &value)
{
	if(m_context == kInvalidThreadHandle) {
		return kEDisposed;
	}

	PRI pri;
	ER er = get_pri(m_context->m_handle, &pri);

	if(er != E_OK) {
		return kfw_convert_er(er);
	}

	value = pri;

	return kOk;
}

void Thread::sleep(uint32_t ms)
{
	kos_dly_tsk(ms);
}

//--- ThreadPool class ---
struct ThreadPoolWorkItem
{
	Callback<void()> m_func;
};

class ThreadPool final : private NonCopyable
{
public:
	ThreadPool() {}

	ret_t start();

	ret_t queue_work_item_isr(const Callback<void()> &func);

private:

	void thread_main();

	Thread m_threads[2];
	DataQueue m_queue;
	FixedMemoryPool m_pool;
};

ret_t ThreadPool::start() {

	m_pool.create(sizeof(ThreadPoolWorkItem), 32);

	m_queue.create(32);

	for(int i = 0; i < 2; i++) {
		Thread &thread = m_threads[i];
		thread.create(Callback<void ()>(this, &ThreadPool::thread_main), kPriorityNormal, 4096);
		thread.start();
	}

	return kOk;
}

void ThreadPool::thread_main() {
	for(;;) {
		uintptr_t data;
		ret_t r = m_queue.recv(data);
		if(is_failed(r)) {
			break;
		}

		ThreadPoolWorkItem *work_item = (ThreadPoolWorkItem *)data;

		work_item->m_func();

		delete work_item;
	}
}

ret_t ThreadPool::queue_work_item_isr(const Callback<void()> &func)
{
	ThreadPoolWorkItem *work_item;
	ret_t r = m_pool.get_isr((void **)&work_item);
	if(is_failed(r)) {
		return r;
	}

	work_item->m_func = func;

	r = m_queue.send_isr((uintptr_t)work_item);
	if(is_failed(r)) {
		m_pool.release(work_item);
		return r;
	}
	return kOk;
}

static ThreadPool m_timer_thread_ppol;

//--- Timer class ---
ret_t Timer::create(const Callback<void()> &func,
		uint32_t interval)
{
	T_CCYC ccyc = {0};
	ccyc.cycatr = 0;
	ccyc.cyctim = interval;
	ccyc.cychdr = (VP)Timer::handler_entry;

	ER_ID er_id = acre_cyc(&ccyc);
	if(er_id != E_OK) {
		return kEUnknown;
	}

	m_handle = er_id;

	return kOk;
}

void Timer::dispose() {
	if(m_handle == 0) return;
	del_cyc(m_handle);
	m_handle = 0;
}

ret_t Timer::start()
{
	sta_cyc(m_handle);
	return kOk;
}
void Timer::stop()
{
	stp_cyc(m_handle);
}

void Timer::handler_entry(Timer *obj)
{
	m_timer_thread_ppol.queue_work_item_isr(obj->m_func);
}

void kfw_rtos_static_init()
{
	m_timer_thread_ppol.start();
}

};};

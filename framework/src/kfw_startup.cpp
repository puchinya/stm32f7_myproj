
#include "kfw_common.hpp"
#include "kfw_system.hpp"
#include "kfw_net_local.hpp"
#include "kfw_rtc.hpp"
#include "kfw_rtos.hpp"
#include <itron.h>
#include <new>

#ifndef CFG_MAIN_THREAD_STACK_SIZE
#define CFG_MAIN_THREAD_STACK_SIZE	4096
#endif

extern "C" {
	void kfw_startup(void);
	void kfw_run(void *p);
};

static uint32_t s_main_stack[CFG_MAIN_THREAD_STACK_SIZE/sizeof(uint32_t)];
static ID s_main_id;

void kfw_startup(void)
{
	kos_ctsk_t ctsk;

	kos_init_kernel();

	kfw::kfw_system_start_cycle_counter();

	ctsk = {0};
	ctsk.task = (kos_fp_t)kfw_run;
	ctsk.stk = s_main_stack;
	ctsk.stksz = CFG_MAIN_THREAD_STACK_SIZE;
	ctsk.tskatr = TA_HLNG | TA_ACT;
	ctsk.itskpri = 1;

	s_main_id = (ID)acre_tsk(&ctsk);

	kos_start_kernel();
}

static void call_constructors()
{
    // Start and end points of the constructor list,
    // defined by the linker script.
    extern void (*__init_array_start)();
    extern void (*__init_array_end)();

    // Call each function in the list.
    // We have to take the address of the symbols, as __init_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)() = &__init_array_start; p < &__init_array_end; ++p) {
        (*p)();
    }
}

void kfw_run(void *p)
{
	extern void app_main();

	kfw::rtos::kfw_rtos_static_init();
	kfw::kfw_rtc_static_init();
	kfw::net::kfw_net_static_init();

	call_constructors();
	app_main();
}

extern "C" void __cxa_pure_virtual()
{
}

void *operator new(std::size_t size)
{
	return kos_alloc(size);
}

void *operator new[](std::size_t size)
{
	return kos_alloc(size);
}

void operator delete(void *p)
{
	kos_free(p);
}

void operator delete[](void *p)
{
	kos_free(p);
}

void operator delete(void *p, std::size_t size)
{
	kos_free(p);
}

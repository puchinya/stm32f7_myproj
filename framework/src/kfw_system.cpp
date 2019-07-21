
#include "kfw_system.hpp"
#include <mcu.h>

namespace kfw {

#define start_timer()    *((volatile uint32_t*)0xE0001000) = 0x40000001  // Enable CYCCNT register
#define stop_timer()   *((volatile uint32_t*)0xE0001000) = 0x40000000  // Disable CYCCNT register
#define get_timer()   *((volatile uint32_t*)0xE0001004)               // Get value from CYCCNT register

void kfw_system_start_cycle_counter()
{
	start_timer();
}

void kfw_system_stop_cycle_counter()
{
	stop_timer();
}

uint32_t kfw_system_get_cycle_counter()
{
	return get_timer();
}

void kfw_system_error(ret_t r)
{
	kfw_system_reset();
}

void kfw_system_reset()
{
	NVIC_SystemReset();
}

};

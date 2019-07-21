
#include "kfw_system.hpp"
#include <mcu.h>

namespace kfw {

void kfw_system_start_cycle_counter()
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void kfw_system_stop_cycle_counter()
{
	DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t kfw_system_get_cycle_counter()
{
	return DWT->CYCCNT;
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

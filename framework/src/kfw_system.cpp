
#include "kfw_system.hpp"
#include <mcu.h>

namespace kfw {

void kfw_system_error(ret_t r)
{
	kfw_system_reset();
}

void kfw_system_reset()
{
	NVIC_SystemReset();
}

};

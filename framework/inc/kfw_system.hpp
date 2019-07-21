#pragma once
#ifndef KFW_SYSTEM_HPP
#define KFW_SYSTEM_HPP

#include "kfw_common.hpp"

namespace kfw
{
	[[noreturn]] void kfw_system_error(ret_t r);
	[[noreturn]] void kfw_system_reset();
	void kfw_system_start_cycle_counter();
	void kfw_system_stop_cycle_counter();
	uint32_t kfw_system_get_cycle_counter();

};

#endif

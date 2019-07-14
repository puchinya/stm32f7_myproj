#pragma once
#ifndef KFW_SYSTEM_HPP
#define KFW_SYSTEM_HPP

#include "kfw_common.hpp"

namespace kfw
{
	[[noreturn]] void kfw_system_error(ret_t r);
	[[noreturn]] void kfw_system_reset();

};

#endif

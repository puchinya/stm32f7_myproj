#pragma once

#include <kfw_netif.hpp>

namespace app {
	using namespace kfw;
	using namespace kfw::net::intf;

	struct GWCheckServiceConfig
	{
		NetworkInterface 	*intf;
		uint32_t			check_interval;	// ms
		uint32_t			error_count;
		Callback<void()>	error_handler;
	};

	class GWCheckService
	{
	private:
		GWCheckServiceConfig	m_cfg;
		void thread_main();
	public:
		ret_t set_config(const GWCheckServiceConfig &cfg);
		ret_t start();
		ret_t stop();
	};
}

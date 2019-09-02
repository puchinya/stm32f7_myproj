#pragma once

#include <kfw_common.hpp>

namespace app {

	using namespace kfw;

	enum class ServiceManagerMessageCode
	{
		kStartup,
		kShutdown,
		kNetworkStatusChanged
	};

	class ServiceMessage;

	class ServiceBase : private NonCopyable
	{
	protected:
		ServiceBase() {}

		ret_t on_start();
		void on_stop();
		ret_t on_event(ServiceMessage *msg);
	};

	class ServiceManager final : private NonCopyable
	{
	public:
		ret_t start();
	};

}

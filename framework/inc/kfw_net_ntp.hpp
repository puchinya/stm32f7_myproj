#pragma once

#include <kfw_common.hpp>
#include <kfw_net.hpp>
#include <kfw_rtos.hpp>

namespace kfw { namespace net {
	class SntpClientService final : private NonCopyable
	{
	public:
		SntpClientService() : m_is_started(false), m_req_quit(false) {}
		ret_t start();
		void stop();

	private:
		void thread_main();
		ret_t do_sync();
		bool m_is_started;
		bool m_req_quit;
		Socket m_socket;
		kfw::rtos::Thread m_thread;
	};
}}

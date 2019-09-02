
#pragma once

#include <kfw_web.hpp>
#include <kfw_rtos.hpp>

namespace app {

	using namespace kfw;
	using namespace kfw::web;
	using namespace kfw::rtos;

	class Httpd : private NonCopyable
	{
	public:
		Httpd() : m_is_running(false) {}

		ret_t start();
		void stop();
		void restart() {
			stop();
			start();
		}
		bool is_running();

	private:
		void run();
		bool m_is_running;
		HttpListener m_http_l;
		Thread m_http_thread;
	};
}

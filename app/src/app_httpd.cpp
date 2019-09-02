
#include "app_httpd.hpp"
#include <kfw_rtc.hpp>

namespace app {

using namespace kfw::io;

ret_t Httpd::start()
{
	if(m_is_running) {
		return kEInvalidOperation;
	}

	ret_t r;

	r = m_http_l.create(80);
	if(is_failed(r)) {
		return r;
	}

	r = m_http_l.start();
	if(is_failed(r)) {
		goto error_failed_http_l_start;
	}

	r = m_http_thread.create(Callback<void()>(this, &Httpd::run), kPriorityNormal, 4096);
	if(is_failed(r)) {
		goto error_failed_thread_create;

	}
	r = m_http_thread.start();
	if(is_failed(r)) {
		goto error_failed_thread_start;
	}

	m_is_running = true;

	return kOk;

error_failed_thread_start:
	m_http_thread.dispose();
error_failed_thread_create:
	m_http_l.stop();
error_failed_http_l_start:
	m_http_l.dispose();

	return r;
}

void Httpd::stop()
{
	if(!m_is_running) {
		return;
	}

	m_http_l.stop();
}

void Httpd::run()
{
	HttpListener &http_l = m_http_l;
	int32_t counter = 0;

	for(;;) {
		HttpListenerContext ctx;

		ret_t r = http_l.get_context(ctx);
		if(is_failed(r)) {
			Thread::sleep(1000);
			continue;
		}

		HttpListenerRequest &req = ctx.get_request();
		HttpListenerResponse &res = ctx.get_response();

		const Uri &uri = req.get_uri();

		if(uri.get_raw_url() == u8"/") {
			auto & res_stream = res.get_stream();
			res.set_status_code(200);
			StreamWriter writer(res_stream);
			DateTime now_time;
			DateTime::get_now(now_time);
			writer << now_time.get_year() << "/" << now_time.get_month() << "/" << now_time.get_day() << " " << now_time.get_hour() << ":" << now_time.get_min() << ":" << now_time.get_sec();
			counter++;
			res.close();
		} else {
			res.set_status_code(404);
			res.close();
		}
	}
}

}

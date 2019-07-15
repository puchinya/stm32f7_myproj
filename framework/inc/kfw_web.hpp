#pragma once
#ifndef KFW_WEB_HPP
#define KFW_WEB_HPP

#include <kfw_net.hpp>

#if 0
namespace kfw { namespace web {

class HttpClient final : private NonCopyable
{
public:
	ret_t get(const ConstStringRef &url);
};

class HttpListenerRequest final : private NonCopyable
{
public:
	const ConstStringRef &get_method() const;
	const ConstStringRef &get_content_type() const;
	const uint32_t get_content_length() const;
	bool get_keep_alive() const {
		return m_keep_alive;
	}
private:
	String m_method;
	bool m_keep_alive;
};

class HttpListenerResponse final : private NonCopyable
{

};

class HttpListenerContext final : private NonCopyable
{
private:
	kfw::net::Socket m_com_soc;
	HttpListenerContext(kfw::net::Socket &soc);
public:
	HttpListenerContext();
	HttpListenerRequest &get_request();
	HttpListenerResponse &get_response();
};

class HttpListener final : private NonCopyable
{
public:
	HttpListener(): m_soc(), m_port(0), m_req_quit(false), m_is_started(false) {}
	~HttpListener() {
		dispose();
	}

	ret_t create();
	void dispose();

	ret_t start(uint32_t port = 80);
	ret_t stop();

	ret_t get_context(HttpListenerContext &ctx);
private:
	kfw::net::Socket	m_soc;
	uint32_t			m_port;
	bool				m_req_quit;
	bool				m_is_started;
};

}}
#endif
#endif

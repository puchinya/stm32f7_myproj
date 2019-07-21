#pragma once
#ifndef KFW_WEB_HPP
#define KFW_WEB_HPP

#include <kfw_net.hpp>

namespace kfw { namespace web {

class Uri final : private NonCopyable
{
public:
	Uri();
	~Uri();

	ret_t init(const ConstStringRef &uri)
	{
		int32_t schema_end_index = uri.index_of(ConstStringRef("://"));
	}

	void dispose();

	ConstStringRef get_fragment() const;
	ConstStringRef get_Host() const;
private:
	String m_raw_url;
	ConstStringRef m_fragment;
	ConstStringRef m_host;
};

class HttpClient final : private NonCopyable
{
public:
	ret_t get(const ConstStringRef &url);
};

class HttpListenerRequest;

class HttpListenerRequestStream final : public kfw::io::Stream
{
	friend class HttpListener;
	friend class HttpListenerRequest;
public:
	HttpListenerRequestStream()
	: m_base_stream(nullptr), m_content_length(0), m_read_pos(0) {}

	void close() override
	{

	}

	RetVal<uint32_t> read(void *buf, uint32_t size) override
	{
		uint32_t read_size = size;
		uint32_t remain_size = m_content_length - m_read_pos;
		if(read_size > remain_size) {
			read_size = remain_size;
		}
		auto r = m_base_stream->read(buf, size);

		if(is_failed(r.ret)) {
			return r;
		}

		m_read_pos += r.val;

		return r;
	}
	RetVal<uint32_t> write(const void *buf, uint32_t size) override
	{
		return RetVal<uint32_t>(kENotSupported, 0);
	}

private:
	void attach(kfw::io::BufferedStream *base_stream, uint32_t content_length)
	{
		m_base_stream = base_stream;
		m_content_length = content_length;
		m_read_pos = 0;
	}

	kfw::io::BufferedStream *m_base_stream;
	uint32_t m_content_length;
	uint32_t m_read_pos;
};

class HttpListenerRequest final : private NonCopyable
{
	friend class HttpListener;
	friend class HttpListenerContext;
	friend class HttpListenerRequestStream;

private:
public:
	~HttpListenerRequest() {
		dispose();
	}
	void dispose() {}

	const ConstStringRef &get_method() const {
		return m_method;
	}
	const ConstStringRef &get_content_type() const;
	const uint32_t get_content_length() const;
	bool get_keep_alive() const {
		return m_keep_alive;
	}

	const Uri &get_uri() const {
		return m_uri;
	}

private:
	HttpListenerRequest()
	: m_keep_alive(false), m_content_length(0) {}
	String m_method;
	Uri m_uri;
	bool m_keep_alive;
	uint32_t m_content_length;
	HttpListenerRequestStream m_stream;

};

class HttpListenerResponse final : private NonCopyable
{
	enum class Stage {
		Init,
		StatusLineWritten,
		HeaderWritten,
		ContentWritten
	};
public:
	void dispose() {}
	//HttpListenerResponseStream m_stream;
};

class HttpListener;

class HttpListenerContext final : private NonCopyable
{
	friend class HttpListener;

public:
	HttpListenerContext() {

	}

	~HttpListenerContext() {
		dispose();
	}

	void dispose() {
		m_client.dispose();
		m_stream.dispose();
		m_request.dispose();
		m_response.dispose();
	}

	HttpListenerRequest &get_request() {
		return m_request;
	}
	HttpListenerResponse &get_response() {
		return m_response;
	}

private:
	kfw::net::TcpClient m_client;
	kfw::io::BufferedStream m_stream;
	kfw::StaticArray<uint8_t, 256> m_buffer;
	HttpListenerRequest m_request;
	HttpListenerResponse m_response;
};

class HttpListener final : private NonCopyable
{
public:
	HttpListener() : m_server(), m_port(0), m_req_quit(false), m_is_started(false) {}
	~HttpListener() {
		dispose();
	}

	ret_t create(int32_t port = 80);
	void dispose();

	ret_t start();
	ret_t stop();

	ret_t get_context(HttpListenerContext &ctx);
private:
	kfw::net::TcpServer	m_server;
	uint32_t			m_port;
	bool				m_req_quit;
	bool				m_is_started;
};

}}

#endif

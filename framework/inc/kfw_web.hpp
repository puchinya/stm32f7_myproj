#pragma once
#ifndef KFW_WEB_HPP
#define KFW_WEB_HPP

#include <kfw_net.hpp>

namespace kfw { namespace web {

class Uri final : private NonCopyable
{
public:
	Uri() {

	}
	~Uri() {
		dispose();
	}

	ret_t init(const ConstStringRef &uri);

	void dispose();

	const ConstStringRef get_raw_url() const {
		return m_raw_url;
	}
	const ConstStringRef get_fragment() const;
	const ConstStringRef get_Host() const;
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
class HttpListenerResponse;

class HttpListenerRequestStream final : public kfw::io::Stream
{
	friend class HttpListener;
	friend class HttpListenerRequest;
public:
	HttpListenerRequestStream()
	: m_base_stream(nullptr), m_content_length(0), m_read_pos(0) {}

	void close() override;

	RetVal<uint32_t> read(void *buf, uint32_t size) override;

	RetVal<uint32_t> write(const void *buf, uint32_t size) override;

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
public:
	~HttpListenerRequest() {
		dispose();
	}

	void dispose() {}

	ConstStringRef get_method() const {
		return m_method;
	}
	const ConstStringRef &get_content_type() const;
	uint32_t get_content_length() const {
		return m_content_length;
	}

	const Uri &get_uri() const {
		return m_uri;
	}

private:
	HttpListenerRequest()
	: m_content_length(0) {}

	String m_method;
	Uri m_uri;
	uint32_t m_content_length;
	HttpListenerRequestStream m_stream;
};

struct ChunkBufferChunk {
	ChunkBufferChunk *m_next;
	uint8_t m_buffer[1];
};


class ChunkBuffer;
class ChunkBufferEnumerator;

class ChunkBuffer final : private NonCopyable
{
	friend class ChunkBufferEnumerator;
public:
	ChunkBuffer()
	: m_first_chunk(nullptr), m_current_chunk(nullptr), m_current_chunk_pos(0), m_chunk_size(0),
	  m_total_size(0) {}
	~ChunkBuffer() {
		dispose();
	}
	ret_t init(uint32_t chunk_size);

	void dispose();

	ret_t write(const void *data, uint32_t len);

	uint32_t get_total_size() const {
		return m_total_size;
	}

private:
	ChunkBufferChunk	*m_first_chunk;
	ChunkBufferChunk	*m_current_chunk;
	uint32_t	m_current_chunk_pos;
	uint32_t	m_chunk_size;
	uint32_t	m_total_size;
};

class ChunkBufferEnumerator : private NonCopyable
{
	friend class ChunkBuffer;
public:
	explicit ChunkBufferEnumerator(ChunkBuffer *cb)
	: m_chunk_buffer(cb), m_current_chunk(cb->m_first_chunk)
	{

	}

	/**
	 * 末尾に達しているか、調べます。
	 */
	bool is_end() const {
		return m_current_chunk == nullptr ? true : false;
	}

	void next () {
		if(m_current_chunk == nullptr)
			return;
		m_current_chunk = m_current_chunk->m_next;
	}

	const uint8_t *get_data() const {
		return m_current_chunk->m_buffer;
	}

	uint32_t get_data_size() const {
		if(m_current_chunk == m_chunk_buffer->m_current_chunk) {
			return m_chunk_buffer->m_current_chunk_pos;
		} else {
			return m_chunk_buffer->m_chunk_size;
		}
	}

private:

	ChunkBuffer *m_chunk_buffer;
	ChunkBufferChunk *m_current_chunk;
};

class HttpListenerContext;

class HttpListenerResponseStream final : public kfw::io::Stream
{
	friend class HttpListenerResponse;
	friend class HttpListener;
public:

	void close() override;

	RetVal<uint32_t> read(void *buf, uint32_t size) override;

	RetVal<uint32_t> write(const void *buf, uint32_t size) override;

private:

	HttpListenerResponseStream(HttpListenerResponse *response)
	: m_response(response) {

	}

	ret_t create();

	HttpListenerResponse *m_response;
	ChunkBuffer m_chunk_buffer;
};

class HttpListenerResponse final : private NonCopyable
{
	friend class HttpListenerContext;
	friend class HttpListener;
public:
	void dispose() {

	}

	void close() {
		if(!m_is_opened) return;

		write_response();

		m_is_opened = false;
	}

	int32_t get_status_code() const {
		return m_status_code;
	}

	void set_status_code(int32_t value) {
		m_status_code = value;
	}

	const String &get_content_type() const {
		return m_content_type;
	}

	void set_content_type(const ConstStringRef &value) {
		m_content_type = value;
	}

	HttpListenerResponseStream &get_stream()
	{
		return m_stream;
	}

private:
	explicit HttpListenerResponse(HttpListenerContext *context)
	:  m_is_opened(false), m_context(context), m_status_code(200), m_stream(this)
	{
	}

	ret_t write_response();

	const char *kCrLf = "\r\n";

	bool m_is_opened;
	HttpListenerContext *m_context;
	int32_t m_status_code;
	String m_content_type;
	HttpListenerResponseStream m_stream;
};

class HttpListener;

class HttpListenerContext final : private NonCopyable
{
	friend class HttpListenerRequest;
	friend class HttpListenerResponse;
	friend class HttpListener;

public:
	HttpListenerContext()
	: m_is_created(false), m_response(this) {
	}

	~HttpListenerContext() {
		dispose();
	}

	void dispose() {
		if(!m_is_created) return;

		m_request.dispose();
		m_response.dispose();
		m_stream.dispose();
		m_client.dispose();
		m_is_created = false;
	}

	HttpListenerRequest &get_request() {
		return m_request;
	}

	HttpListenerResponse &get_response() {
		return m_response;
	}

private:
	bool m_is_created;
	kfw::net::TcpClient m_client;
	kfw::io::BufferedStream m_stream;
	kfw::StaticArray<uint8_t, 256> m_buffer;
	HttpListenerRequest m_request;
	HttpListenerResponse m_response;
};

class HttpListener final : private NonCopyable
{
public:
	HttpListener() : m_server(), m_port(0), m_is_started(false) {}
	~HttpListener() {
		dispose();
	}

	ret_t create(int32_t port = 80);
	void dispose();

	ret_t start();
	void stop();

	ret_t get_context(HttpListenerContext &ctx);
private:
	kfw::net::TcpServer	m_server;
	uint32_t			m_port;
	bool				m_is_started;
};

class HttpUtil final : private NonCopyable
{
public:
	static const char8_t *get_status_code_text(int32_t status_code);
private:
	HttpUtil() {}
};

}}

#endif

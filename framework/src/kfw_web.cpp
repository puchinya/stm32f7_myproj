
#include "kfw_web.hpp"
#include "kfw_text.hpp"

using namespace kfw;
using namespace kfw::io;
using namespace kfw::net;
using namespace kfw::text;

namespace kfw { namespace web {

//--- Uri class ---
ret_t Uri::init(const ConstStringRef &uri)
{
	m_raw_url = uri;
	//int32_t schema_end_index = m_raw_url.index_of(ConstStringRef("://"));
	return kOk;
}

void Uri::dispose()
{

}

//--- ChunkBuffer class ---

ret_t ChunkBuffer::init(uint32_t chunk_size) {
	if(m_first_chunk != nullptr) {
		return kEInvalidOperation;
	}
	m_chunk_size = chunk_size;
	m_current_chunk_pos = 0;
	m_first_chunk = (ChunkBufferChunk *)new uint8_t[chunk_size + offsetof(ChunkBufferChunk, m_buffer)];
	if(!m_first_chunk) {
		return kEOutOfMemory;
	}
	m_first_chunk->m_next = nullptr;
	m_current_chunk = m_first_chunk;
	m_total_size = 0;
	return kOk;
}

void ChunkBuffer::dispose() {
	ChunkBufferChunk *chunk = m_first_chunk;
	if(chunk == nullptr) return;
	for(; chunk != nullptr; ) {
		ChunkBufferChunk *next_chunk = chunk->m_next;
		delete chunk;
		chunk = next_chunk;
	}
	m_first_chunk = nullptr;
	m_current_chunk = nullptr;
}

ret_t ChunkBuffer::write(const void *data, uint32_t len) {

	uint32_t remain = len;
	const uint8_t *p = (const uint8_t *)data;

	while(remain > 0) {
		uint32_t current_remain = m_chunk_size - m_current_chunk_pos;
		if(current_remain == 0) {
			ChunkBufferChunk *new_chunk = (ChunkBufferChunk *)new uint8_t[m_chunk_size + offsetof(ChunkBufferChunk, m_buffer)];
			if(new_chunk == nullptr) {
				return kEOutOfMemory;
			}
			m_current_chunk->m_next = new_chunk;
			m_current_chunk = new_chunk;
			m_current_chunk_pos = 0;
			continue;
		}

		uint32_t copy_size = remain;
		if(copy_size > current_remain) {
			copy_size = current_remain;
		}

		memcpy(&m_current_chunk->m_buffer[m_current_chunk_pos], p, copy_size);
		p += copy_size;
		remain -= copy_size;
		m_current_chunk_pos += copy_size;
		m_total_size += copy_size;
	}

	return kOk;
}

//--- HttpListenerRequestStream class ---

void HttpListenerRequestStream::close()
{
	// read remain contents
	uint8_t buf[64];
	while(m_read_pos < m_content_length)
	{
		uint32_t rem = m_content_length - m_read_pos;
		if(rem > 64) rem = 64;
		auto r = m_base_stream->read(buf, rem);
		if(r.is_failed()) {
			return;
		}
	}
}

RetVal<uint32_t> HttpListenerRequestStream::read(void *buf, uint32_t size)
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

RetVal<uint32_t> HttpListenerRequestStream::write(const void *buf, uint32_t size)
{
	return RetVal<uint32_t>(kENotSupported, 0);
}

//--- HttpListenerResponseStream class ---
ret_t HttpListenerResponseStream::create() {
	m_chunk_buffer.init(512);
	return kOk;
}

void HttpListenerResponseStream::close()
{

}

RetVal<uint32_t> HttpListenerResponseStream::read(void *buf, uint32_t size)
{
	return RetVal<uint32_t>(kENotSupported, 0);
}

RetVal<uint32_t> HttpListenerResponseStream::write(const void *buf, uint32_t size)
{
	ret_t r = m_chunk_buffer.write(buf, size);
	if(is_failed(r)) {
		return RetVal<uint32_t>(r, 0);
	}
	return RetVal<uint32_t>(kOk, size);
}

//--- HttpListenerResponse class ---

ret_t HttpListenerResponse::write_response()
{
	// Status Line
	auto &stream = m_context->m_stream;
	StreamWriter writer(stream);

	// write status line
	writer << "HTTP/1.1 " << m_status_code << " " << HttpUtil::get_status_code_text(m_status_code) << kCrLf;
	ret_t r = writer.get_error();
	if(is_failed(r)) {
		return r;
	}
	stream.flush();

	// write headers
	writer << "Content-Length: " << m_stream.m_chunk_buffer.get_total_size() << kCrLf;
	writer << "Content-Type: " << m_content_type << kCrLf;
	writer << "Connection: close" << kCrLf;
	writer << kCrLf;
	r = writer.get_error();
	if(is_failed(r)) {
		return r;
	}
	stream.flush();

	{
		ChunkBufferEnumerator cb_enum(&m_stream.m_chunk_buffer);

		while(!cb_enum.is_end())
		{
			auto write_ret = stream.write(cb_enum.get_data(), cb_enum.get_data_size());
			if(is_failed(write_ret.ret)) {
				return r;
			}

			cb_enum.next();
		}
	}

	stream.flush();

	// 書き込んだので解放する。
	m_stream.m_chunk_buffer.dispose();

	return kOk;
}


//--- HttpListener class ---

ret_t HttpListener::create(int32_t port)
{
	ret_t r = m_server.create(port);
	return r;
}

void HttpListener::dispose()
{
	m_server.dispose();
}

ret_t HttpListener::start()
{
	if(m_is_started) {
		return kEInvalidOperation;
	}

	ret_t r = m_server.start();
	if(is_failed(r)) {
		return r;
	}

	m_is_started = true;

	return kOk;
}

void HttpListener::stop()
{
	if(!m_is_started) {
		return;
	}

	m_is_started = false;
}

ret_t read_line(BufferedStream &s, StringBuffer &line)
{
	line.clear();

	uint8_t buf[1];
	for(;;) {
		uint8_t b;
		ret_t r = s.read(buf, 1).ret;
		b = buf[0];
		if(b == '\r') {
			uint8_t b2;
			r = s.read(buf, 1).ret;
			b2 = buf[0];
			if(b2 == '\n') {
				break;
			}

			r = line.append(b);
			if(is_failed(r)) {
				return r;
			}

			r = line.append(b2);
			if(is_failed(r)) {
				return r;
			}
		} else {
			r = line.append(b);
			if(is_failed(r)) {
				return r;
			}
		}
	}

	return kOk;
}

ret_t parse_request_line(const ConstStringRef &request_line,
		ConstStringRef &method, ConstStringRef &url, ConstStringRef &http_version)
{
	int32_t p0 = request_line.index_of(' ');

	if(p0 < 0) {
		return kEFormat;
	}

	int32_t p1 = request_line.index_of(' ', p0 + 1);

	method = request_line.sub_string(0, p0);
	if(p1 >= 0) {
		url = request_line.sub_string(p0 + 1, p1 - (p0 + 1));
		http_version = request_line.sub_string(p1 + 1);
	} else {
		url = request_line.sub_string(p0 + 1);
		http_version = ConstStringRef();
	}

	return kOk;
}

ret_t parse_http_header(const StringBuffer &sb, ConstStringRef &key, ConstStringRef &val)
{
	ConstStringRef header = sb;

	int32_t offset = header.index_of(':');
	if(offset < 0) {
		return kEArgument;
	}

	key = header.sub_string(0, offset).trim();
	val = header.sub_string(offset + 1).trim();

	return kOk;
}

ret_t HttpListener::get_context(HttpListenerContext &ctx)
{
	if(!m_is_started) {
		return kEInvalidOperation;
	}

	if(ctx.m_is_created) {
		return kEArgument;
	}

	// ソケットをaccept
	auto &server = m_server;
	StringBuffer line(1024);
	ret_t r;

	r = server.accept_tcp_client(ctx.m_client);
	if(is_failed(r)) {
		return r;
	}

	auto &client = ctx.m_client;
	auto &bs = ctx.m_stream;

	bs.init(client.get_stream(), ctx.m_buffer, ctx.m_buffer.get_byte_size(), nullptr, 0);

	// read Request-Line
	r = read_line(bs, line);
	if(is_failed(r)) {
		return r;
	}

	// parse Request-Line
	ConstStringRef method, url, http_version;
	r = parse_request_line(line, method, url, http_version);
	if(is_failed(r)) {
		return r;
	}

	ctx.m_request.m_method = method;
	ctx.m_request.m_uri.init(url);

	uint32_t content_length = 0;
	bool found_content_length = false;

	// read and parse Headers
	for(;;) {
		r = read_line(bs, line);
		if(is_failed(r)) {
			return r;
		}
		if(line.get_length() == 0) {
			break;
		}

		ConstStringRef key;
		ConstStringRef val;
		ret_t rr = parse_http_header(line, key, val);
		if(rr == kOk) {
			if(key.compare_ignore_case("Content-Length") == 0) {
				uint32_t content_length;
				r = UInt32::parse(val, content_length);
				if(r == kOk) {
					found_content_length = true;
				}
			}
		}
	}

	ctx.m_request.m_content_length = content_length;
	ctx.m_request.m_stream.attach(&ctx.m_stream, content_length);

	ctx.m_response.m_status_code = 200;
	ctx.m_response.m_content_type = u8"text/plain";
	ctx.m_response.m_stream.create();
	ctx.m_response.m_is_opened = true;

	ctx.m_is_created = true;

	return kOk;
}

const char8_t *HttpUtil::get_status_code_text(int32_t status_code)
{
	switch(status_code) {
	case 200:
		return u8"OK";
	case 201:
		return u8"Created";
	case 202:
		return u8"Accepted";
	case 400:
		return u8"Bad Request";
	case 401:
		return u8"Unauthorized";
	case 403:
		return u8"Forbidden";
	case 404:
		return u8"Not Found";
	case 405:
		return u8"Method Not Allowed";
	case 500:
		return u8"Internal Server Error";
	default:
		return u8"Unknown";
	}
}

}}


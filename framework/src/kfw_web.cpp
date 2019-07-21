
#include "kfw_web.hpp"
#include "kfw_text.hpp"

using namespace kfw;
using namespace kfw::io;
using namespace kfw::net;
using namespace kfw::text;

namespace kfw { namespace web {
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

ret_t read_line(BufferedStream &s, StringBuffer &line)
{
	uint8_t buf[1];
	for(;;) {
		uint8_t b;
		ret_t r = s.read(buf, 1).ret;
		b = buf[0];
		if(b == '¥r') {
			uint8_t b2;
			r = s.read(buf, 1).ret;
			b2 = buf[0];
			if(b2 == '¥n') {
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
		method = request_line.sub_string(p0 + 1, p1);
		http_version = request_line.sub_string(p1 + 1);
	} else {
		method = request_line.sub_string(p0 + 1);
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

class UInt32 final
{
public:
	UInt32(uint32_t value) : m_value(value) {}
	UInt32() : m_value(0) {}

	static ret_t parse(const ConstStringRef &s, uint32_t &val);

private:
	uint32_t	m_value;
};

ret_t UInt32::parse(const ConstStringRef &s, uint32_t &val)
{
	if(s.get_length() == 0) {
		return kEFormat;
	}

	uint32_t r = 0;
	for(int p = 0; p < s.get_length();p++)
	{
		int32_t ch = s[p];
		if(ch < '0' || ch > '9') {
			return kEFormat;
		}
		int n = ch - '0';
		r = r *10 + n;
	}
	return r;
}



ret_t HttpListener::get_context(HttpListenerContext &ctx)
{
	if(!m_is_started) {
		return kEInvalidOperation;
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

	ConstStringRef method, url, http_version;
	r = parse_request_line(line, method, url, http_version);
	if(is_failed(r)) {
		return r;
	}

	ctx.m_request.m_method = method;

	int32_t status = 200;
	uint32_t content_length = 0;
	bool found_content_length = false;

	// read Headers
	for(;;) {
		r = read_line(bs, line);
		if(line.get_length() == 0) {
			break;
		}

		ConstStringRef key;
		ConstStringRef val;
		ret_t rr = parse_http_header(line, key, val);
		if(rr == kOk) {
			if(key.compare_ignore_case("ContentLength") == 0) {
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

	return kOk;
}
}}


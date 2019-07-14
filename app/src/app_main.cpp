
#include <kfw_netif.hpp>
#include <kfw_net.hpp>
#include <kfw_stm_netif.hpp>
#include <kfw_rtos.hpp>
#include <kfw_system.hpp>

using namespace kfw;
using namespace kfw::net;
using namespace kfw::net::intf;
using namespace kfw::net::intf::stm;
using namespace kfw::rtos;

#if 0
class String final
{
public:

};

class StringBuffer final : private NonCopyable
{
public:
	StringBuffer(size_t capacity);
	~StringBuffer();

	char8_t at(uint32_t index) const {
		if(index >= m_length) {
			kfw_system_error(kEIndexOutOfRange);
		}
		return m_data[index];
	}

	ret_t append(int ch);
private:
	size_t		m_capacity;
	size_t		m_length;
	char8_t		*m_data;
};

StringBuffer::StringBuffer(size_t capacity)
{
	char8_t *mem = new char8_t[capacity];

	m_capacity = capacity;
	m_length = 0;
	m_data = mem;
}

StringBuffer::~StringBuffer()
{
	if(m_data != nullptr) {
		delete [] m_data;
		m_data = nullptr;
	}
}

ret_t StringBuffer::append(int ch)
{
	if(m_length >- m_capacity)
		return kEOutOfMemory;
	m_data[m_length++] = (char8_t)ch;

	return kOk;
}

class HttpRequest
{

};

class HttpResponse
{

};

#define CFG_HTTP_LISTENER_MAX_HEADER_BUF_SIZE	4096
#define CFG_HTTP_LISTENER_MAX_


class ConstStringRef final
{
public:
	explicit ConstStringRef(const char8_t *c_str)
	{
		m_data = c_str;
		m_length = strlen(c_str);
	}

	ConstStringRef(const char8_t *data, uint32_t length) :
		m_data(data), m_length(length)
	{
	}

	const char8_t *get_data() const {
		return m_data;
	}
	const uint32_t get_length() const {
		return m_length;
	}
private:
	const char8_t	*m_data;
	uint32_t		m_length;
};

class StringRef final
{
public:
	StringRef(char8_t *c_str)
	{
		m_data = c_str;
		m_length = strlen(c_str);
	}
	StringRef(char8_t *data, uint32_t length) :
		m_data(data), m_length(length)
	{

	}

	ConstStringRef get_const_ref() const {
		return ConstStringRef(m_data, m_length);
	}
private:
	char8_t		*m_data;
	uint32_t	m_length;
};

class HttpListenerHeaderItem
{
private:
	ConstStringRef	m_name;
	ConstStringRef	m_value;
};

class HttpListenerHeader
{
private:

	uint8_t	m_header_buffer[CFG_HTTP_LISTENER_MAX_HEADER_BUF_SIZE];
};

class HttpListenerContext
{
private:
	Socket m_com_soc;
	HttpListenerContext(Socket &soc);
public:
	HttpListenerContext();
	HttpRequest &get_request();
	HttpResponse &get_response();
};

class HttpListener final : private kfw::NonCopyable
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
	Socket		m_soc;
	uint32_t	m_port;
	bool		m_req_quit;
	bool		m_is_started;
	void thread_main();
};

ret_t HttpListener::create()
{
	ret_t r = m_soc.open(SocketType::kTCP);
	return r;
}

void HttpListener::dispose()
{
	m_soc.close();
}

ret_t HttpListener::start(uint32_t port)
{
	if(m_is_started) {
		return kEInvalidOperation;
	}

	m_port = port;

	auto &soc = m_soc;

	ret_t r = soc.bind(SocketAddress(IPAddress::get_Loopback(), m_port));
	if(r != kOk) {
		return r;
	}
	r = soc.listen(16);
	if(r != kOk) {
		return r;
	}

	m_is_started = true;

	return kOk;
}

class BufferedStream final : public kfw::io::Stream
{
public:
	BufferedStream();

	ret_t create(kfw::io::Stream *base_stream, uint8_t *buffer, uint32_t buffet_size);
	void dispose();
	int read_byte();
private:
	kfw::io::Stream *m_base_stream;
	uint8_t *m_buffer;
	uint32_t m_buffer_size;
};

ret_t read_line(BufferedStream &s, StringBuffer &line)
{
	for(;;) {
		int b = s.read_byte();
		if(b == '¥r') {
			int b2 = s.read_byte();
			if(b2 == '¥n') {
				break;
			}

			ret_t r = line.append(b);
			if(is_failed(r)) {
				return r;
			}

			ret_t r = line.append(b2);
			if(is_failed(r)) {
				return r;
			}
		} else {
			ret_t r = line.append(b);
			if(is_failed(r)) {
				return r;
			}
		}
	}

	return kOk;
}

ret_t HttpListener::get_context(HttpListenerContext &ctx)
{
	// ソケットをaccept
	auto &soc = m_soc;
	StringBuffer line(1024);
	BufferedStream bs;
	uint8_t buf[256];

	ret_t r;

	r = soc.accept(ctx.m_com_soc);
	if(is_failed(r)) {
		return r;
	}

	auto &com_soc = ctx.m_com_soc;

	com_soc.set_opt_recv_timeout(5000);
	com_soc.set_opt_send_timeout(5000);

	SocketStream ss(com_soc);

	bs.create(&ss, buf, sizeof(buf));

	// read Request-Line
	r = read_line(bs, line);
	if(is_failed(r)) {
		//
	}

	// read Headers
	for(;;) {
		r = read_line(bs, line);
		if(line.get_length() == 0) {
			break;
		}
	}

	return kOk;
}

#if 0

class LocalApiGateway final
{
public:
	ret_t start();
	ret_t stop();
	ret_t register_api_ep(const char *ep);
};

DEFINE_API_ROUTE(config_api_route, "/config", config_api_handler);

void config_api_handler()
{

}

void httpd_thread()
{
	HttpServer server;

	ret_t r = server.start();
	KFW_ASSERT(r == kOk, r);
	for(;;) {
		HttpServerContext ctx;
		server.get_context(ctx);
		auto req = ctx.get_request();

		req.get_raw_url()
	}
}

enum class TimeSyncedMode
{
	kNone,
	kSntp,
	kManual
};

using time_t = uint64_t;

class TimeSyncService
{
public:
	ret_t start();
	ret_t stop();

	bool is_time_synced();
	ret_t manual_sync(time_t tm);
	TimeSyncedMode get_time_synced_mode();

	RetVal<time_t> get_synced_time();
};

ret_t cfg_set(uint32_t id, void *d, size_t s);
ret_t cfg_get_info(uint32_t id);
ret_t cfg_save();
ret_t cfg_load();
ret_t cfg_default_and_save();

cfg_set<int>(CFG_NAME(ST), 60);

/*
 *	<sys-cfg>
 *		<cfg name="ST" id="1" type="int" minlen=1, maxlen=1 default="5"/>
 *	</sys-cfg>
 *
 */

void time_sync()
{

}

class AutoResetService
{
public:
	ret_t start();
	ret_t stop();


};

void auto_reset()
{
	uint32_t cnt = 24 * 3;
	for(;;) {
		Thread::sleep(60*60*1000);
		if(--cnt == 0) {
			break;
		}
	}

	kfw_system_reset();
}
#endif

struct GWCheckServiceConfig
{
	IEMACInterface 		*intf;
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
#endif

static Thread test_thread;
static uint8_t http_buffer[4096];

void test_func()
{
	TcpClient tcp;
/*
	ret_t r;
	r = tcp.create();
	//r = tcp.connect("yahoo.co.jp", 80);
	r = tcp.connect("192.168.3.13", 80);
	kfw::io::Stream *s = tcp.get_stream();
	const char *req = "GET / HTTP/1.1\r\nHost: yahoo.co.jp\r\nConnection: close\r\n\r\n";
	s->write(req, strlen(req));
	auto read_r = s->read(http_buffer, sizeof(http_buffer));
	http_buffer[read_r.val] = 0;
	tcp.disconnect();*/
	return;
}

void app_main()
{
	auto nif = STMEhternetInterface::get_instance();
	nif->set_use_dhcp(true);
	nif->connect();

	// 接続完了を待つ
	IPAddress addr;
	for(;;) {
		nif->get_ip(addr);
		if(nif->get_connection_status() == ConnectionStatus::kLocalUp) {
			break;
		}
		Thread::sleep(1000);
	}

	test_thread.create(test_func, 1, 2048);
	test_thread.start();
}

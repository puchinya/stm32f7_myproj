
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
using namespace kfw::io;

class String final
{
public:

};

#define CFG_HTTP_LISTENER_MAX_HEADER_BUF_SIZE	4096
#define CFG_HTTP_LISTENER_MAX_



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
	NetworkInterface 	*intf;
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

void netinterface_status_changed(NetworkInterface *nif)
{
	if(nif->get_connection_status() == ConnectionStatus::kConnected) {
		// Start or resume services
	} else {
		// Stop or suspend services
	}
}

enum class ServiceManagerMessageCode
{
	kStartup,
	kShutdown,
	kNetworkStatusChanged
};

class ServiceMessage;

class ServiceBase
{
protected:
	void on_start();
	void on_stop();
	void on_event(ServiceMessage *msg);
};

struct ServiceManagerMessage
{
	ServiceManagerMessageCode	msg_code;
	union {
		struct {
			NetworkInterface *inf;
		} network_status_change;
	} data;
};

class ServiceManager final : private NonCopyable
{

private:

	void thread_main();

	ret_t send_message();

	Thread m_thread;
	DataQueue m_msg_queue;
	MemoryPool m_msg_mem_pool;
};




void app_main()
{
	auto nif = STMEhternetInterface::get_instance();
	nif->set_use_dhcp(true);
	nif->connect();

	// 接続完了を待つ
	for(;;) {
		if(nif->get_connection_status() == ConnectionStatus::kConnected) {
			break;
		}
		Thread::sleep(1000);
	}

	test_thread.create(test_func, 1, 2048);
	test_thread.start();
}

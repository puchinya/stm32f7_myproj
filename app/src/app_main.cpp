
#include <kfw_netif.hpp>
#include <kfw_net.hpp>
#include <kfw_net_security.hpp>
#include <kfw_stm_netif.hpp>
#include <kfw_rtos.hpp>
#include <kfw_system.hpp>
#include <kfw_web.hpp>
#include <kfw_net_ntp.hpp>

using namespace kfw;
using namespace kfw::net;
using namespace kfw::net::intf;
using namespace kfw::net::intf::stm;
using namespace kfw::rtos;
using namespace kfw::io;
using namespace kfw::web;

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

/*
using WebApiHandler = Callback<ret_t, >

void add_auth(const ConstStringRef &rel_uri, AuthHandler *handler);
void register_api(const ConstStringRef &rel_uri, WebApiHandler *web_api_handler);
*/

const char8_t *g_ca_pem = u8"-----BEGIN CERTIFICATE-----"
		"MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ"
		"RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD"
		"VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX"
		"DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y"
		"ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy"
		"VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr"
		"mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr"
		"IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK"
		"mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu"
		"XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy"
		"dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye"
		"jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1"
		"BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3"
		"DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92"
		"9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx"
		"jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0"
		"Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz"
		"ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS"
		"R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp"
		"-----END CERTIFICATE-----";

void Http_client()
{
	TlsStream tls;
	TcpClient client;
	X509Certificate ca_cert;

	ret_t r;

	r = ca_cert.create(g_ca_pem, strlen(g_ca_pem) + 1);
	if(is_failed(r)) {
		return;
	}

	r = client.create();
	if(is_failed(r)) {
		return;
	}
	r = client.connect("yahoo.co.jp", 443);
	if(is_failed(r)) {
		return;
	}
	r = tls.create(client.get_stream());
	if(is_failed(r)) {
		return;
	}
	r = tls.authenticate_as_client(ConstStringRef("yahoo.co.jp"), &ca_cert);
	if(is_failed(r)) {
		return;
	}
	return;
}

void Http_server()
{
	HttpListener httpL;
	httpL.create(80);
	httpL.start();
	for(;;) {
		HttpListenerContext ctx;

		ret_t r = httpL.get_context(ctx);
		/*
		if(is_failed(t)) {
			break;
		}

		HttpListenerRequest &req = ctx.get_request();
		HttpListenerRequest &res = ctx.get_response();

		if(req.get_url() == "/TestDef.html") {
			ctx.get_response().get_stream().write("TEST");
		}*/
	}
}

static SntpClientService s_sntp;

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

	s_sntp.start();

	//test_thread.create(test_func, 1, 2048);
	//test_thread.start();
	//Http_client();
}

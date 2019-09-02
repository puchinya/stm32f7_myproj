
#include <kfw_netif.hpp>
#include <kfw_net.hpp>
#include <kfw_net_security.hpp>
#include <kfw_stm_netif.hpp>
#include <kfw_rtos.hpp>
#include <kfw_system.hpp>
#include <kfw_web.hpp>
#include <kfw_net_ntp.hpp>
#include <stdio.h>
#include <kfw_rtc.hpp>
#include "app_httpd.hpp"

using namespace kfw;
using namespace kfw::net;
using namespace kfw::net::intf;
using namespace kfw::net::intf::stm;
using namespace kfw::rtos;
using namespace kfw::io;
using namespace kfw::web;
using namespace app;

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

const char8_t *g_ca_pem = u8"-----BEGIN CERTIFICATE-----\n"
		"MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G\n"
		"A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp\n"
		"Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1\n"
		"MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG\n"
		"A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n"
		"hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL\n"
		"v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8\n"
		"eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq\n"
		"tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd\n"
		"C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa\n"
		"zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB\n"
		"mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH\n"
		"V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n\n"
		"bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG\n"
		"3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs\n"
		"J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO\n"
		"291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS\n"
		"ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd\n"
		"AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7\n"
		"TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==\n"
		"-----END CERTIFICATE-----\n";

void Http_client()
{
	TlsStream tls;
	TcpClient client;
	X509Certificate ca_cert;

	ret_t r;

	auto prev_tm = Rtc::get_time();
	r = ca_cert.create(g_ca_pem, strlen(g_ca_pem) + 1);
	if(is_failed(r)) {
		return;
	}

	r = client.create();
	if(is_failed(r)) {
		return;
	}
	r = client.connect("google.co.jp", 443);
	if(is_failed(r)) {
		return;
	}

	client.get_socket().set_nonblocking(false);

	r = tls.create(client.get_stream());
	if(is_failed(r)) {
		return;
	}
	r = tls.authenticate_as_client(ConstStringRef("google.co.jp"), &ca_cert);
	auto next_tm = Rtc::get_time();
	auto dt =  (int32_t)(next_tm - prev_tm);
	if(is_failed(r)) {
		printf("Failed Auth %d\n", dt);
		return;
	}
	printf("SUCCESS %d\n", dt);
	return;
}




static SntpClientService s_sntp;
static Httpd s_httpd;

enum class EventLogEntryType
{
	kInfo = 0,
	kWarning = 1,
	kError = 2,
};

class EventLog : private NonCopyable
{
public:

	void set_source(const char8_t *value);
	void write_entry(const char8_t *source, const char *message, EventLogEntryType type);
	void write_entry(const char *message, EventLogEntryType type);
	void write_entry(const char *message);
};

enum class AppMessageCode
{
	NetworkConnected,
	TimeSynced,
	NetworkDisconnected,
	SystemReset
};

struct AppMessage {
	AppMessageCode	code;
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

	s_sntp.start();
	s_httpd.start();

	// wait for time sync
	for(;;) {
		if(Rtc::is_synced()) {
			break;
		}
		Thread::sleep(1000);
	}

	//test_thread.create(Http_client, kPriorityNormal, 8192);
	//test_thread.start();
}

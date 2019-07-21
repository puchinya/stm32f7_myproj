
#include "kfw_net_ntp.hpp"
#include "kfw_rtc.hpp"

namespace kfw { namespace net {

using namespace kfw::rtos;

constexpr int kSntpPort = 123;
constexpr uint32_t kNtpUtcOffset = 2208988800;

ret_t SntpClientService::start()
{
	ret_t r;

	Callback<void()> cb(this, &SntpClientService::thread_main);
	r = m_thread.create(cb, kPriorityLow, 2048);
	if(is_failed(r)) {
		return r;
	}

	r = m_socket.open(SocketType::kUDP);
	if(is_failed(r)) {
		m_thread.dispose();
		return r;
	}

	m_socket.set_recv_timeout(10 * 1000);
	m_socket.set_send_timeout(10 * 1000);

	m_socket.bind(SocketAddress(IPAddress::get_Any(), kSntpPort));

	m_req_quit = false;

	r = m_thread.start();

	m_is_started = true;

	return kOk;
}

void SntpClientService::stop()
{

}

void SntpClientService::thread_main()
{
	uint8_t data[48] = {0};
	uint8_t recv_data[64] = {0};
	SocketAddress soc_addr;
	ret_t r;

	while(!m_req_quit) {
		data[0] = 0xE3;
		data[1] = 0;
		data[2] = 6;
		data[3] = 0xEC;

		IPAddress addr;

		r = Dns::get_host_by_name("ntp.nict.jp", addr);
		if(is_failed(r)) {
			Thread::sleep(5000);
			continue;
		}

		RetVal<uint32_t> ur = m_socket.send_to(data, sizeof(data), SocketAddress(addr, kSntpPort));
		if(is_failed(ur.ret)) {
			Thread::sleep(5000);
			continue;
		}

		SocketAddress remote_addr;

		ur = m_socket.recv_from(recv_data, sizeof(recv_data), remote_addr);
		if(is_failed(ur.ret)) {
			Thread::sleep(5000);
			continue;
		}

		uint32_t t0 = recv_data[40];
		uint32_t t1 = recv_data[41];
		uint32_t t2 = recv_data[42];
		uint32_t t3 = recv_data[43];
		uint32_t sec = (t0 << 24) | (t1 << 16) | (t2 << 8) | t3;
		sec = sec - kNtpUtcOffset; // To UTC

		Rtc::sync((uint64_t)sec * 1000);

		Thread::sleep(10 * 60 * 1000);
	}
}

}}

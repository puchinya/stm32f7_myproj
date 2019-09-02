
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

ret_t SntpClientService::do_sync()
{
	uint8_t send_data[48] = {0};
	uint8_t recv_data[48];
	SocketAddress soc_addr;
	ret_t r;

	send_data[0] = 0xE3;
	send_data[1] = 0;
	send_data[2] = 6;
	send_data[3] = 0xEC;

	IPAddress addr;

	r = Dns::get_host_by_name("ntp.nict.jp", addr);
	if(is_failed(r)) {
		return r;
	}

	UdpClient client;
	r = client.create(0);
	if(is_failed(r)) {
		return r;
	}

	client.get_socket().set_recv_timeout(5000);

	int ok_cnt = 0;
	int error_cnt = 0;

	utc_time_t offsets[4];

	for(;;) {

		utc_time_t cli_send_tm = Rtc::get_time();

		RetVal<uint32_t> rr = client.send(send_data, sizeof(send_data),
				SocketAddress(addr, kSntpPort));
		if(rr.is_failed()) {
			error_cnt++;
			if(error_cnt >= 3) {
				r = rr.ret;
				return r;
			}
			Thread::sleep(5000);
			continue;
		}

		SocketAddress remote;

		rr = client.recv(recv_data, sizeof(recv_data),
				remote);
		if(rr.is_failed()) {
			error_cnt++;
			if(error_cnt >= 3) {
				r = rr.ret;
				return r;
			}
			Thread::sleep(5000);
			continue;
		}
		utc_time_t cli_recv_tm = Rtc::get_time();

		uint32_t rtm0 = recv_data[36];
		uint32_t rtm1 = recv_data[37];
		uint32_t rtm2 = recv_data[38];
		uint32_t rtm3 = recv_data[39];
		uint32_t srv_recv_tm_msec = (rtm0 << 24) | (rtm1 << 16) | (rtm2 << 8) | rtm3;
		srv_recv_tm_msec = ((uint64_t)srv_recv_tm_msec * 1000) >> 32;

		uint32_t rt0 = recv_data[32];
		uint32_t rt1 = recv_data[33];
		uint32_t rt2 = recv_data[34];
		uint32_t rt3 = recv_data[35];
		uint32_t srv_recv_tm_sec = (rt0 << 24) | (rt1 << 16) | (rt2 << 8) | rt3;
		utc_time_t srv_recv_tm = (((utc_time_t)srv_recv_tm_sec - kNtpUtcOffset) * 1000 + srv_recv_tm_msec);

		uint32_t stm0 = recv_data[44];
		uint32_t stm1 = recv_data[45];
		uint32_t stm2 = recv_data[46];
		uint32_t stm3 = recv_data[47];
		uint32_t srv_send_tm_msec = (stm0 << 24) | (stm1 << 16) | (stm2 << 8) | stm3;
		srv_send_tm_msec = ((uint64_t)srv_send_tm_msec * 1000) >> 32;

		uint32_t st0 = recv_data[40];
		uint32_t st1 = recv_data[41];
		uint32_t st2 = recv_data[42];
		uint32_t st3 = recv_data[43];
		uint32_t srv_send_tm_sec = (st0 << 24) | (st1 << 16) | (st2 << 8) | st3;
		utc_time_t srv_send_tm = (((utc_time_t)srv_send_tm_sec - kNtpUtcOffset) * 1000 + srv_send_tm_msec);

		utc_time_t offset = (srv_recv_tm - cli_send_tm) / 2 - (cli_recv_tm - srv_send_tm) / 2;

		offsets[ok_cnt] = offset;

		ok_cnt++;
		if(ok_cnt >= 4) {
			break;
		}

		Thread::sleep(100);
	}

	client.dispose();

	utc_time_t offset_avg = offsets[0] / 4 + offsets[1] / 4 + offsets[2] / 4 + offsets[3] / 4;
	utc_time_t cur_tm = Rtc::get_time();
	cur_tm += offset_avg;
	Rtc::sync(cur_tm);

	return kOk;
}

void SntpClientService::thread_main()
{
#if 0
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

		if(ur.val < 48) {
			Thread::sleep(5000);
			continue;
		}

		uint32_t t0 = recv_data[40];
		uint32_t t1 = recv_data[41];
		uint32_t t2 = recv_data[42];
		uint32_t t3 = recv_data[43];
		uint32_t sec = (t0 << 24) | (t1 << 16) | (t2 << 8) | t3;
		if(sec < kNtpUtcOffset) {
			Thread::sleep(5000);
			continue;
		}
		sec = sec - kNtpUtcOffset; // To UTC

		Rtc::sync((uint64_t)sec * 1000);

		Thread::sleep(10 * 60 * 1000);
	}
#endif
	for(;;) {
		if(is_failed(do_sync())) {
			Thread::sleep(30 * 1000);
		} else {
			Thread::sleep(10 * 60 * 1000);
		}
	}
}

}}

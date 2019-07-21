
#include "kfw_net_local.hpp"
#include <lwip/tcpip.h>
#include "kfw_rtos.hpp"
#include <lwip/dhcp.h>
#include <lwip/igmp.h>
#include <lwip/ip.h>

namespace kfw { namespace net
{

void net_addr_to_lwip_addr(const IPAddress &from, ip_addr_t &to)
{
	to.addr = from.get_value();
}

void lwip_addr_to_net_addr(const ip_addr_t &from, IPAddress &to)
{
	to.set_value(from.addr);
}

/**
 * @TODO
 */
ret_t lwip_err_to_ret(err_t err)
{
	switch(err) {
	case ERR_OK: return kOk;
	case ERR_MEM: return kEOutOfMemory;
	case ERR_BUF: return kEOutOfMemory;
	case ERR_TIMEOUT: return kETimeout;
	case ERR_RTE: return kEInvalidOperation; // zantei
	case ERR_VAL: return kEArgument; // zantei
	case ERR_WOULDBLOCK: return kEInvalidOperation; //zantei
	case ERR_USE: return kEInvalidOperation; // zantei
	case ERR_ALREADY: return kEInvalidOperation; // zanntei
	case ERR_ISCONN: return kEInvalidOperation; //zantei
	case ERR_CONN: return kEInvalidOperation; // zantei
	case ERR_IF: return kEInvalidOperation; // zantei
	case ERR_ABRT: return kESocketAborted;
	case ERR_RST: return kESocketConnectionReset;
	case ERR_CLSD: return kESocketClosed;
	case ERR_ARG: return kEArgument;
	default:
		return kEInvalidOperation;
	}
}

class SocketBuffer final : private NonCopyable
{
	friend Socket;
private:
	struct netbuf *m_handle;
public:
	SocketBuffer()
	: m_handle(nullptr)
	{

	}
	~SocketBuffer() {
		dispose();
	}

	void dispose();

	ret_t alloc(size_t size);
	ret_t free();
	ret_t ref(const void *buf, size_t size);
};

void SocketBuffer::dispose()
{
	if(m_handle == nullptr)
		return;
	netbuf_delete(m_handle);
	m_handle = nullptr;
}

ret_t SocketBuffer::ref(const void *buf, size_t size)
{
	if(buf == nullptr || size > 0xffff) {
		return kEArgument;
	}

	if(m_handle == nullptr) {
		m_handle = netbuf_new();
		if(m_handle == nullptr) {
			return kEOutOfMemory;
		}
	}

	err_t err;
	err = netbuf_ref(m_handle, buf, (u16_t)size);
	return lwip_err_to_ret(err);
}

class SocketContext final : private NonCopyable
{
	friend class Socket;
public:
	SocketContext() : m_handle(kInvalidSocketHandle), m_recv_buf(nullptr), m_recv_buf_offset(0)
	{}

	~SocketContext()
	{
		dispose();
	}

	void dispose();
private:
	socket_handle_t m_handle;
	struct netbuf *m_recv_buf;
	uint32_t m_recv_buf_offset;
};

void SocketContext::dispose()
{
	if(m_recv_buf != nullptr) {
		netbuf_delete(m_recv_buf);
		m_recv_buf = nullptr;
	}
}

ret_t Socket::open(SocketType socket_type)
{
	if(m_context != nullptr) {
		return kEInvalidOperation;
	}

	enum netconn_type nc_type;

	switch(socket_type) {
	case SocketType::kRAW:
		nc_type = NETCONN_RAW;
		break;
	case SocketType::kUDP:
		nc_type = NETCONN_UDP;
		break;
	case SocketType::kTCP:
		nc_type = NETCONN_TCP;
		break;
	default:
		return kEArgument;
	}

	SocketContext *context = new SocketContext();
	if(context == nullptr) {
		return kEOutOfMemory;
	}

	auto handle = netconn_new(nc_type);

	if(handle == kInvalidSocketHandle) {
		delete context;
		return kEResource;
	}

	context->m_handle = handle;

	m_context = context;

	return kOk;
}

void Socket::close()
{
	if(m_context == nullptr) {
		return;
	}

	netconn_close(m_context->m_handle);
	m_context->m_handle = kInvalidSocketHandle;
	delete m_context;
	m_context = nullptr;
}

ret_t Socket::connect(const char8_t *host, int32_t port)
{
	IPAddress addr;
	ret_t r;

	r = Dns::get_host_by_name(host, addr);
	if(is_failed(r)) {
		return r;
	}

	return connect(addr, port);
}

ret_t Socket::connect(const IPAddress &addr, int32_t port)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	err_t err;
	ip_addr_t ip;

	net_addr_to_lwip_addr(addr, ip);

	err = netconn_connect(m_context->m_handle, &ip, (u16_t)port);

	return lwip_err_to_ret(err);
}

ret_t Socket::disconnect()
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	err_t err;

	err = netconn_disconnect(m_context->m_handle);

	return lwip_err_to_ret(err);
}

ret_t Socket::bind(const SocketAddress &addr)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	err_t err;

	ip_addr_t ip;

	net_addr_to_lwip_addr(addr.get_address(), ip);

	err = netconn_bind(m_context->m_handle, &ip, (u16_t)addr.get_port());

	return lwip_err_to_ret(err);
}

ret_t Socket::listen(int32_t backlogs)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	auto err = netconn_listen_with_backlog(m_context->m_handle, (u8_t)backlogs);
	return lwip_err_to_ret(err);
}

ret_t Socket::accept(Socket &socket)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	if(socket.m_context != nullptr) {
		return kEArgument;
	}

	socket_handle_t accepted_socket;

	SocketContext *context = new SocketContext();
	if(context == nullptr) {
		return kEOutOfMemory;
	}

	err_t err;

	err = netconn_accept(m_context->m_handle, &accepted_socket);
	if(err != ERR_OK) {
		delete context;
		return lwip_err_to_ret(err);
	}

	context->m_handle = accepted_socket;

	socket.m_context = context;

	socket.set_keep_alive(true);

	return kOk;
}


RetVal<uint32_t> Socket::recv_from(void *buf, uint32_t size, SocketAddress &remote)
{
	if(m_context == nullptr) {
		return RetVal<uint32_t>(kEDisposed, 0);
	}
	if(buf == nullptr) {
		return RetVal<uint32_t>(kEArgument, 0);
	}
	if(size == 0) {
		return RetVal<uint32_t>(kOk, 0);
	}

	err_t err = ERR_OK;
	uint8_t *buf_ptr = (uint8_t *)buf;
	size_t remain_read_size = size;

	while(remain_read_size > 0) {

		if(m_context->m_recv_buf != nullptr) {
			void *p;
			u16_t len;
			err = netbuf_data(m_context->m_recv_buf, &p, &len);
			if(err != ERR_OK) {
				break;
			}
			size_t remain_size = (size_t)len - m_context->m_recv_buf_offset;
			size_t copy_size = size;
			if(copy_size > remain_size) copy_size = remain_size;
			memcpy(buf_ptr, (uint8_t *)p + m_context->m_recv_buf_offset, copy_size);
			buf_ptr += copy_size;
			m_context->m_recv_buf_offset += copy_size;
			remain_read_size -= copy_size;
			remote.m_address.m_address = m_context->m_recv_buf->addr.addr;
			remote.m_port = m_context->m_recv_buf->port;
			if(m_context->m_recv_buf_offset == len) {
				netbuf_delete(m_context->m_recv_buf);
				m_context->m_recv_buf = nullptr;
			}
			break;
		} else {
			struct netbuf *recv_buf;
			err = netconn_recv(m_context->m_handle, &recv_buf);
			if(err != ERR_OK) {
				break;
			}
			m_context->m_recv_buf_offset = 0;
			m_context->m_recv_buf = recv_buf;
		}
	}

	ret_t r = lwip_err_to_ret(err);

	return RetVal<uint32_t>(r, size - remain_read_size);
}

RetVal<uint32_t> Socket::send_to(const void *buf, uint32_t size, const SocketAddress &remote)
{
	if(m_context == nullptr) {
		return RetVal<uint32_t>(kEDisposed, 0);
	}
	if(buf == nullptr) {
		return RetVal<uint32_t>(kEArgument, 0);
	}
	if(size == 0) {
		return RetVal<uint32_t>(kOk, 0);
	}

	if(m_context->m_handle->type == NETCONN_TCP) {
		size_t written_size;
		err_t err = netconn_write_partly(m_context->m_handle, buf, size, NETCONN_NOCOPY, &written_size);

		ret_t r = lwip_err_to_ret(err);
		if(is_failed(r)) {
			return RetVal<uint32_t>(r, 0);
		}
		return RetVal<uint32_t>(r, (uint32_t)written_size);
	} else {

		SocketBuffer sock_buf;
		ret_t r;
		r = sock_buf.ref(buf, size);
		if(r != kOk) {
			return RetVal<uint32_t>(r, 0);
		}

		ip4_addr addr;
		addr.addr = remote.get_address().get_value();

		err_t err = netconn_sendto(m_context->m_handle, sock_buf.m_handle, &addr, remote.m_port);

		r = lwip_err_to_ret(err);
		if(is_failed(r)) {
			return RetVal<uint32_t>(r, 0);
		}
		return RetVal<uint32_t>(r, size);
	}
}

ret_t Socket::add_membership(const IPAddress &value)
{
	ip4_addr addr;
	addr.addr = value.m_address;
	err_t err = netconn_join_leave_group(m_context->m_handle,
			&addr, nullptr, NETCONN_JOIN);

	return lwip_err_to_ret(err);
}

ret_t Socket::drop_membership(const IPAddress &value)
{
	ip4_addr addr;
	addr.addr = value.m_address;
	err_t err = netconn_join_leave_group(m_context->m_handle,
			&addr, nullptr, NETCONN_LEAVE);

	return lwip_err_to_ret(err);
}

ret_t Socket::set_keep_alive(bool value)
{
	if(value) {
		ip_set_option(m_context->m_handle->pcb.ip, SOF_KEEPALIVE);
	} else {
		ip_reset_option(m_context->m_handle->pcb.ip, SOF_KEEPALIVE);
	}

	return kOk;
}

ret_t Socket::get_keep_alive(bool &value)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	value = ip_get_option(m_context->m_handle->pcb.ip, SOF_KEEPALIVE) != 0;
	return kOk;
}

ret_t Socket::set_recv_timeout(int32_t value)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	netconn_set_recvtimeout(m_context->m_handle, value);

	return kOk;
}

ret_t Socket::get_recv_timeout(int32_t &value)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	value = netconn_get_recvtimeout(m_context->m_handle);

	return kOk;
}

ret_t Socket::set_send_timeout(int32_t value)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	netconn_set_sendtimeout(m_context->m_handle, value);

	return kOk;
}
ret_t Socket::get_send_timeout(int32_t &value)
{
	if(m_context == nullptr) {
		return kEDisposed;
	}

	value = netconn_get_sendtimeout(m_context->m_handle);

	return kOk;
}

// --- SocketStream class ---
SocketStream::~SocketStream()
{
}

void SocketStream::flush()
{

}

void SocketStream::close()
{
	m_socket.disconnect();
}

RetVal<uint32_t> SocketStream::read(void *buf, uint32_t size)
{
	return m_socket.recv(buf, size);
}

RetVal<uint32_t> SocketStream::write(const void *buf, uint32_t size)
{
	return m_socket.send(buf, size);
}

TcpClient::TcpClient()
 : m_socket(), m_stream(m_socket)
{

}

TcpClient::~TcpClient()
{
	dispose();
}

ret_t TcpClient::create(AddressFamily addressFamily) {
	ret_t r = m_socket.open(SocketType::kTCP);

	if(is_failed(r)) {
		return r;
	}

	r = m_socket.set_keep_alive(true);
	if(is_failed(r)) {
		return r;
	}

	r = m_socket.set_send_timeout(30 * 1000);
	if(is_failed(r)) {
		return r;
	}

	r = m_socket.set_recv_timeout(30 * 1000);
	if(is_failed(r)) {
		return r;
	}

	return kOk;
}

void TcpClient::dispose() {
	m_socket.close();
}

ret_t TcpClient::connect(const char8_t *host, int32_t port)
{
	return m_socket.connect(host, port);
}

ret_t TcpClient::connect(const IPAddress &addr, int32_t port)
{
	return m_socket.connect(addr, port);
}

ret_t TcpClient::disconnect()
{
	return m_socket.disconnect();
}

ret_t Dns::get_host_by_name(const char8_t *host_name, IPAddress &address)
{
	ip_addr_t addr;
	err_t err = netconn_gethostbyname(host_name, &addr);
	address.m_address = addr.addr;
	return lwip_err_to_ret(err);
}

//--- TcpServer class ---
ret_t TcpServer::create(int32_t port)
{
	ret_t r = m_socket.open(SocketType::kTCP);
	if(is_failed(r)) {
		return r;
	}
	r = m_socket.bind(SocketAddress(IPAddress::get_Loopback(), port));
	if(is_failed(r)) {
		return r;
	}

	return kOk;
}

ret_t TcpServer::start(int32_t backlogs)
{
	return m_socket.listen(backlogs);
}

ret_t TcpServer::accept_tcp_client(TcpClient &client)
{
	return m_socket.accept(client.m_socket);
}

//--- functions ---
/*
void kfw_net_lwip_timer(void)
{
	int dhcp_cd = 0;
	int igmp_cd = 0;
	int dhcp_coarse_cd = 0;

	for(;;)
	{
		// 500ms
		if(dhcp_cd == 0) {
			dhcp_fine_tmr();
			dhcp_cd = 5;
		}
		// 100ms
		if(igmp_cd == 0) {
			igmp_tmr();
			igmp_cd = 1;
		}
		// 1 minute
		if(dhcp_coarse_cd == 0) {
			dhcp_coarse_tmr();
			dhcp_coarse_cd = 600;
		}
		kfw::rtos::Thread::sleep(100);
		dhcp_cd--;
		igmp_cd--;
		dhcp_coarse_cd--;
	}
}

static kfw::rtos::Thread s_kfw_net_lwip_timer_thread;
*/

void kfw_net_static_init(void)
{
	tcpip_init(NULL, NULL);

	//s_kfw_net_lwip_timer_thread.create(kfw_net_lwip_timer, 1, 1024, nullptr);

	//s_kfw_net_lwip_timer_thread.start();
}

}}

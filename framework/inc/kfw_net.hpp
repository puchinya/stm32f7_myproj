
#pragma once
#ifndef KFW_NET_HPP
#define KFW_NET_HPP

#include <lwip/api.h>
#include <kfw_net_common.hpp>
#include <kfw_io.hpp>
#include <kfw_netif.hpp>

namespace kfw { namespace net
{

	constexpr component_id_t kComponentNet = 2;

	constexpr ret_t kESocketConnectionReset = make_err_ret(kComponentNet, 1);
	constexpr ret_t kESocketAborted = make_err_ret(kComponentNet, 2);
	constexpr ret_t kESocketClosed = make_err_ret(kComponentNet, 3);

	class SocketAddress
	{
		friend class Socket;
	private:
		IPAddress	m_address;
		int			m_port;
	public:
		SocketAddress() :
					m_address(), m_port(0)
		{

		}

		SocketAddress(const IPAddress &address, int port) :
			m_address(address), m_port(port)
		{

		}

		SocketAddress(const SocketAddress &obj)
		{
			m_address = obj.m_address;
			m_port = obj.m_port;
		}

		const IPAddress &get_address() const {
			return m_address;
		}
		int get_port() const {
			return m_port;
		}
	};

	enum class SocketType
	{
		kRAW,
		kUDP,
		kTCP,
	};

	typedef struct netconn *socket_handle_t;
	constexpr socket_handle_t kInvalidSocketHandle = nullptr;
	class SocketContext;

	class Socket final : private NonCopyable
	{
	public:
		Socket() : m_context(nullptr) {}
		Socket(Socket &&obj) {
			m_context = obj.m_context;
			obj.m_context = nullptr;
		}
		~Socket() {
			close();
		}

		ret_t open(SocketType socket_type);
		void close();

		ret_t connect(const char8_t *host, int32_t port);
		ret_t connect(const IPAddress &addr, int32_t port);
		ret_t connect(const SocketAddress &sock_addr) {
			return connect(sock_addr.get_address(), sock_addr.get_port());
		}
		ret_t disconnect();
		ret_t bind(const SocketAddress &addr);
		ret_t listen(int32_t backlogs = 16);
		// 未初期化のSocketを渡してください。
		// 未初期化とはopenが実施されていないまたはopen後にcloseを実施済みのもの
		ret_t accept(Socket &socket);
		RetVal<uint32_t> recv(void *buf, uint32_t size) {
			SocketAddress remote;
			return recv_from(buf, size, remote);
		}
		RetVal<uint32_t> send(const void *buf, uint32_t size) {
			SocketAddress remote(IPAddress(), 0);
			return send_to(buf, size, remote);
		}
		RetVal<uint32_t> recv_from(void *buf, uint32_t size, SocketAddress &remote);
		RetVal<uint32_t> send_to(const void *buf, uint32_t size, const SocketAddress &remote);
		ret_t select();

		// Socket options
		ret_t add_membership(const IPAddress &value);
		ret_t drop_membership(const IPAddress &value);
		ret_t set_keep_alive(bool value);
		ret_t get_keep_alive(bool &value);
		ret_t set_recv_timeout(int32_t value);
		ret_t get_recv_timeout(int32_t &value);
		ret_t set_send_timeout(int32_t value);
		ret_t get_send_timeout(int32_t &value);

		Socket &&move() {
			return (Socket &&)*this;
		}

		Socket & operator = (Socket &&obj) {
			close();
			m_context = obj.m_context;
			obj.m_context = nullptr;
			return *this;
		}
	private:
		SocketContext *m_context;
	};

	class SocketStream final : public kfw::io::Stream
	{
	public:
		SocketStream(Socket &socket)
		: m_socket(socket)
		{
		}

		~SocketStream() override;

		void close() override;

		RetVal<uint32_t> read(void *buf, uint32_t size) override;
		RetVal<uint32_t> write(const void *buf, uint32_t size) override;

	private:
		Socket &m_socket;
	};

	class TcpServer;

	class TcpClient final : private NonCopyable
	{
		friend class TcpServer;
	public:
		TcpClient();
		~TcpClient();

		ret_t create(AddressFamily addressFamily = AddressFamily::kInterNetwork);
		void dispose();

		ret_t connect(const char8_t *host, int32_t port);
		ret_t connect(const IPAddress &addr, int32_t port);
		ret_t connect(const SocketAddress &sock_addr) {
			return connect(sock_addr.get_address(), sock_addr.get_port());
		}

		ret_t disconnect();

		SocketStream *get_stream() {
			return &m_stream;
		}

		ret_t set_recv_timeout(int32_t value) {
			return m_socket.set_recv_timeout(value);
		}
		ret_t get_recv_timeout(int32_t &value) {
			return m_socket.get_recv_timeout(value);
		}
		ret_t set_send_timeout(int32_t value) {
			return m_socket.set_send_timeout(value);
		}
		ret_t get_send_timeout(int32_t &value) {
			return m_socket.get_send_timeout(value);
		}

	private:
		Socket m_socket;
		SocketStream m_stream;
	};

	class TcpServer final : private NonCopyable
	{
	public:
		TcpServer() : m_socket() {}
		~TcpServer() = default;

		ret_t create(int32_t port);
		void dispose() {
			m_socket.close();
		}

		ret_t start(int32_t backlogs = 16);

		ret_t accept_tcp_client(TcpClient &client);

	private:
		Socket m_socket;
	};

	class Dns final : private NonCopyable
	{
	private:
		Dns() {}
	public:
		static ret_t get_host_by_name(const char8_t *host_name, IPAddress &address);
	};

	enum class DnsSdProtocol
	{
		kUdp = 0,
		kTcp = 1
	};
/*
	class MDnsResponder final : private NonCopyable
	{
	private:
		MDnsResponder();
		static MDnsResponder m_instance;
	public:
		static MDnsResponder *get_instance();

		ret_t start();
		ret_t stop(); // not supported

		ret_t add_interface(kfw::net::intf::NetworkInteerface *intf, const char *host_name, uint32_t dns_ttl = 7);
		ret_t remove_interface(kfw::net::intf::NetworkInteerface *intf);

		ret_t add_service(kfw::net::intf::NetworkInteerface *intf,
				const char *name, const char *service, DnsSdProtocol protocol, const char *txt_record = nullptr);
	};*/
}}

#endif

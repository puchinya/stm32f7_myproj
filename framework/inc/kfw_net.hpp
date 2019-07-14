
#pragma once
#ifndef KFW_NET_HPP
#define KFW_NET_HPP

#include <kfw_common.hpp>
#include <kfw_io.hpp>
#include <lwip/api.h>

namespace kfw { namespace net
{

	class Socket;
	class Dns;
	constexpr component_id_t kComponentNet = 2;

	constexpr ret_t kESocketConnectionReset = make_err_ret(kComponentNet, 1);
	constexpr ret_t kESocketAborted = make_err_ret(kComponentNet, 2);
	constexpr ret_t kESocketClosed = make_err_ret(kComponentNet, 3);

	constexpr uint32_t pack_ip_address(int a0, int a1, int a2, int a3)
	{
		return ((uint32_t)a3 << 24) |
				((uint32_t)a2 << 16) |
				((uint32_t)a1 << 8) |
				((uint32_t)a0 << 0);
	}

	enum class AddressFamily
	{
		kInterNetwork,
		kInterNetworkV6
	};

	class IPAddress
	{
		friend class Dns;
		friend class Socket;
	private:
		AddressFamily m_address_family;
		uint32_t m_address;

	public:
		IPAddress(int a0, int a1, int a2, int a3)
		: m_address_family(AddressFamily::kInterNetwork), m_address(pack_ip_address(a0, a1, a2, a3))
		{
		}

		IPAddress()
		: m_address_family(AddressFamily::kInterNetwork), m_address(0)
		{

		}

		IPAddress(const IPAddress &obj)
		{
			m_address_family = obj.m_address_family;
			m_address = obj.m_address;
		}

		uint32_t get_value() const {
			return m_address;
		}

		void set_value(uint32_t value) {
			m_address = value;
		}

		AddressFamily get_address_family() const {
			return AddressFamily::kInterNetwork;
		}

		static IPAddress get_Any() {
			return IPAddress(0, 0, 0, 0);
		}
		static IPAddress get_Broadcast() {
			return IPAddress(255, 255, 255, 255);
		}
		static IPAddress get_Loopback() {
			return IPAddress(127, 0, 0, 1);
		}
		static IPAddress get_None() {
			return IPAddress(255, 255, 255, 255);
		}
	};

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
		ret_t set_opt_keep_alive(bool value);
		ret_t get_opt_keep_alive(bool &value);
		ret_t set_opt_multicast_interface(const IPAddress &value);
		ret_t set_opt_multicast_ttl(int32_t value);
		ret_t set_opt_recv_timeout(int32_t value);
		ret_t get_opt_recv_timeout(int32_t &value);
		ret_t set_opt_send_timeout(int32_t value);
		ret_t get_opt_send_timeout(int32_t &value);

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

		RetVal<uint32_t> read(void *buf, size_t size) override;
		RetVal<uint32_t> write(const void *buf, size_t size) override;

	private:
		Socket &m_socket;
	};

	class TcpClient final : private NonCopyable
	{
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
	private:
		Socket m_socket;
		SocketStream m_stream;
	};

	class TcpServer final : private NonCopyable
	{
	public:

	};

	class Dns final : private NonCopyable
	{
	private:
		Dns() {}
	public:
		static ret_t get_host_by_name(const char *host_name, IPAddress &address);
	};
}}

#endif

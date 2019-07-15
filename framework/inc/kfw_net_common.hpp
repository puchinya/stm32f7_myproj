
#pragma once
#ifndef KFW_NET_COMMON_HPP
#define KFW_NET_COMMON_HPP

#include <kfw_common.hpp>

namespace kfw { namespace net {

	class Socket;
	class Dns;

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
}}

#endif


#pragma once

#include "kfw_netif.hpp"
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>

namespace kfw { namespace net { namespace intf { namespace stm {

	class STMEhternetInterface : public IEMACInterface
	{
	public:
		ret_t set_use_dhcp(bool value) override {
			m_use_dhcp = value;
			return kOk;
		}
		ret_t get_use_dhcp(bool &value) override {
			value = m_use_dhcp;
			return kOk;
		}
		ret_t get_ip(IPAddress &value) override;
		ret_t connect() override;
		ret_t disconnect() override;
		ConnectionStatus get_connection_status() override;

		static STMEhternetInterface *get_instance();
	private:
		static STMEhternetInterface m_instance;
		STMEhternetInterface();
		bool m_use_dhcp;
		bool m_is_connected;
		struct netif m_netif;
		struct dhcp m_dhcp;
	};

};};};};

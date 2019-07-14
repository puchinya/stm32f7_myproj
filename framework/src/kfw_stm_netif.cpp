
#include "kfw_stm_netif.hpp"
#include "kfw_net_local.hpp"

namespace kfw { namespace net { namespace intf { namespace stm {

	extern "C" {
		extern err_t ethernetif_init(struct netif *netif);
	};

	STMEhternetInterface::STMEhternetInterface()
	: m_use_dhcp(true), m_is_connected(false)
	{

	}

	STMEhternetInterface STMEhternetInterface::m_instance;

	STMEhternetInterface *STMEhternetInterface::get_instance()
	{
		return &m_instance;
	}

	ret_t STMEhternetInterface::get_ip(IPAddress &value)
	{
		kfw::net::lwip_addr_to_net_addr(m_netif.ip_addr, value);
		return kOk;
	}

	ret_t STMEhternetInterface::connect()
	{
		ip_addr_t ipaddr;
		ip_addr_t netmask;
		ip_addr_t gw;
		err_t err;

		if(m_use_dhcp) {
			IP4_ADDR(&ipaddr, 0, 0, 0, 0);
			IP4_ADDR(&netmask, 0, 0, 0, 0);
			IP4_ADDR(&gw, 0, 0, 0, 0);
		} else {
			IP4_ADDR(&ipaddr, 0, 0, 0, 0);
			IP4_ADDR(&netmask, 0, 0, 0, 0);
			IP4_ADDR(&gw, 0, 0, 0, 0);
		}

		(void)netif_add(&m_netif, &ipaddr, &netmask, &gw, (void *)this,
				ethernetif_init, tcpip_input);

		netif_set_default(&m_netif);

		if(netif_is_link_up(&m_netif))
		{
			netif_set_up(&m_netif);
		} else {
			netif_set_down(&m_netif);
		}

		if(m_use_dhcp) {
			dhcp_set_struct(&m_netif, &m_dhcp);

			err = dhcp_start(&m_netif);
			if(err != ERR_OK) {
				// Error
			}
		}

		return kOk;
	}

	ret_t STMEhternetInterface::disconnect()
	{
		if(m_use_dhcp) {
			dhcp_stop(&m_netif);
			dhcp_cleanup(&m_netif);
		}
		netif_remove(&m_netif);

		return kOk;
	}

	ConnectionStatus STMEhternetInterface::get_connection_status()
	{
		if(m_netif.ip_addr.addr != 0) {
			return ConnectionStatus::kLocalUp;
		}
		return ConnectionStatus::kDisconnected;
	}

};};};};

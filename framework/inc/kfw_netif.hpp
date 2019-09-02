
#pragma once

#include <kfw_net_common.hpp>

namespace kfw { namespace net { namespace intf {

	class NetworkInterface;

	enum class ConnectionStatus
	{
		kDisconnected,
		kConnecting,
		kConnected,
	};

	interface INetworkInterfaceEventListener
	{
	public:
		virtual ~INetworkInterfaceEventListener() {}
		virtual void on_link_chnaged(NetworkInterface *sender) = 0;
		virtual void on_status_changed(NetworkInterface *sender) = 0;
	};

	class NetworkInterface : private NonCopyable
	{
	public:
		virtual ~NetworkInterface();

		ret_t add_event_listener(INetworkInterfaceEventListener *listener);

		ret_t remove_event_listener(INetworkInterfaceEventListener *listener);

		ret_t set_use_dhcp(bool value)
		{
			m_use_dhcp = value;
			return kOk;
		}
		ret_t get_use_dhcp(bool &value)
		{
			value = m_use_dhcp;
			return kOk;
		}

		virtual ret_t get_ip(IPAddress &value) = 0;
		virtual ret_t connect() = 0;
		virtual ret_t disconnect() = 0;
		virtual ConnectionStatus get_connection_status() = 0;
	protected:
		NetworkInterface();

		void fire_link_changed();
		void fire_status_changed();

		bool m_use_dhcp;
	};

};};};

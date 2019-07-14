
#pragma once

#include <kfw_common.hpp>
#include <kfw_net.hpp>

namespace kfw { namespace net { namespace intf {
	class IEMACInterface;

	enum class ConnectionStatus
	{
		kLocalUp,
		kGlobalUp,
		kDisconnected,
		kConnecting
	};

	interface IEMACInterfaceEventListener
	{
	protected:
		IEMACInterfaceEventListener() {}
	public:
		virtual ~IEMACInterfaceEventListener() {}
		virtual void on_link_up(IEMACInterface *sender) = 0;
		virtual void on_link_down(IEMACInterface *sender) = 0;
		virtual void on_up(IEMACInterface *sender) = 0;
		virtual void on_down(IEMACInterface *sender) = 0;
	};

	interface IEMACInterface
	{
	public:
		virtual ~IEMACInterface() {}
		ret_t set_event_listener(IEMACInterfaceEventListener *p_listener);
		virtual ret_t set_use_dhcp(bool value) = 0;
		virtual ret_t get_use_dhcp(bool &value) = 0;
		virtual ret_t get_ip(IPAddress &value) = 0;
		virtual ret_t connect() = 0;
		virtual ret_t disconnect() = 0;
		virtual ConnectionStatus get_connection_status() = 0;
	};

};};};

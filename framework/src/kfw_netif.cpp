
#include <kfw_netif.hpp>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>

namespace kfw { namespace net { namespace intf {

NetworkInterface::NetworkInterface()
: m_use_dhcp(true)
{

}


NetworkInterface::~NetworkInterface()
{

}

ret_t NetworkInterface::add_event_listener(INetworkInterfaceEventListener *listener)
{

}

ret_t NetworkInterface::remove_event_listener(INetworkInterfaceEventListener *listener)
{

}

};};};


#pragma once

#include <kfw_net.hpp>

namespace kfw { namespace net {

void net_addr_to_lwip_addr(const IPAddress &from, ip_addr_t &to);
void lwip_addr_to_net_addr(const ip_addr_t &from, IPAddress &to);
ret_t lwip_err_to_ret(err_t err);
void kfw_net_static_init(void);

};};

#ifndef NETWORKD_H
#define NETWORKD_H

#include <lwip/netif.h>
#include <lwip/tcpip.h>

extern struct netif g_netif;

void networkd_start(void);
void networkd_stop(void);
void networkd_get_ip(ip_addr_t *ip);

#endif

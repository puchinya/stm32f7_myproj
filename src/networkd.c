/*
 * networkd.c
 *
 *  Created on: 2018/03/22
 *      Author: masatakanabeshima
 */
#include "networkd.h"
#if 0

struct netif g_netif;

extern err_t ethernetif_init(struct netif *netif);

static void configure_netif(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  err_t err;

  /* IP address setting */
//  IP4_ADDR(&ipaddr, 192, 168, 100, 200);
//  IP4_ADDR(&netmask, 255, 255, 255, 0);
//  IP4_ADDR(&gw, 192, 168, 100, 1);

  IP4_ADDR(&ipaddr, 0, 0, 0, 0);
  IP4_ADDR(&netmask, 0, 0, 0, 0);
  IP4_ADDR(&gw, 0, 0, 0, 0);

  /* Add a network interface to the list of lwIP netifs. */
  netif_add(&g_netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /*  Registers the default network interface. */
  netif_set_default(&g_netif);

  if (netif_is_link_up(&g_netif))
    {
      /* When the netif is fully configured this function must be called.*/
      netif_set_up(&g_netif);
    }
    else
    {
      /* When the netif link is down this function must be called */
      netif_set_down(&g_netif);
    }

  {
	  extern void ethernetif_update_config(struct netif *netif);
//	  netif_set_link_callback(&g_netif, ethernetif_update_config);
  }
}

static struct dhcp g_dhcp;

static void init_dhcpc(void)
{
	err_t err;

	dhcp_set_struct(&g_netif, &g_dhcp);

	err = dhcp_start(&g_netif);
	if(err != ERR_OK) {
	  LWIP_DEBUGF(LWIP_DBG_ON, ("DHCP failed"));
	}
}

void networkd_start(void)
{
	tcpip_init(NULL, NULL);

	configure_netif();

	init_dhcpc();
}

void networkd_stop(void)
{

}

void networkd_get_ip(ip_addr_t *ip)
{
	*ip = g_netif.ip_addr;
}
#endif

#include "mdnsd.h"
#include "networkd.h"
#include <mdnsd.h>

static void srv_txt(struct mdns_service *service, void *txt_userdata);

/*
void mdnsd_start(void)
{
	err_t err;

	mdns_resp_init();

	err = mdns_resp_add_netif(&g_netif, g_netif.hostname, 3600);
	LWIP_ERROR("mdns add service txt failed\n", (err == ERR_OK), return);
	err = mdns_resp_add_service(&g_netif, "lwipweb", "_http", DNSSD_PROTO_TCP, 80, 3600, srv_txt, NULL);
	LWIP_ERROR("mdns add service txt failed\n", (err == ERR_OK), return);
}

static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
   err_t res = mdns_resp_add_service_txtitem(service, "path=/", 6);
   LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
   LWIP_UNUSED_ARG(txt_userdata);
}
*/

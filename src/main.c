
#include <cmsis_os.h>
#include <mcu.h>
#include <stm32746g_discovery.h>
#include <stm32746g_discovery_lcd.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/api.h>
#include <lwip/apps/mdns.h>
#include <lwip/dhcp.h>
#include <lwip/igmp.h>
#include <ethernetif.h>
#include <networkd.h>
#include <mdnsd.h>

static void SystemClock_Config(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);

static void init_task (void const *arg);
static void led_blink_task (void const *arg);
static void gui_task(void const *arg);
static void httpd_task(void const *arg);
//static void dhcpc_thread(void const *arg);

osThreadDef (led_blink_task, osPriorityHigh, 1, 0);
osThreadDef (httpd_task, osPriorityNormal, 1, 8192);
osThreadDef (init_task, osPriorityRealtime, 1, 4096);
osThreadDef (gui_task, osPriorityLow, 1, 4096);
//osThreadDef (dhcpc_thread, osPriorityLow, 1, 4096);

static void led_blink_task (void const *arg)
{
	BSP_LED_Init(LED1);
	//First thread
	while (1) { // loop forever
		BSP_LED_On(LED1);
		osDelay(500);
		BSP_LED_Off(LED1);
		osDelay(500);
	}
}

err_t ethernetif_init(struct netif *netif);

//static struct dhcp st_dhcp;

static const char s_html[] =
{
	"HTTP/1.1 200 OK\r\n"
	"Connection: close\r\n"
	"Content-Length: 41\r\n"
	"\r\n"
	"<html>"
	"<body>"
	"<h1>Hello</h1>"
	"</body>"
	"</html>"
};


void http_server_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	err_t recv_err;
	char* buf;
	u16_t buflen;
	struct fs_file * file;

	/* Read the data from the port, blocking if nothing yet there.
	We assume the request (the part we care about) is in one netbuf */
	recv_err = netconn_recv(conn, &inbuf);

	if (recv_err == ERR_OK)
	{
		if (netconn_err(conn) == ERR_OK)
		{
			netbuf_data(inbuf, (void**)&buf, &buflen);

			/* Is this an HTTP GET command? (only check the first 5 chars, since
			there are other formats for GET, and we're keeping it very simple )*/
			if ((buflen >=5) && (strncmp(buf, "GET /", 5) == 0))
			{
				netconn_write(conn, (const unsigned char*)(s_html), (size_t)sizeof(s_html), NETCONN_NOCOPY);
			}
		}
	}
	/* Close the connection (server closes in HTTP) */
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
	so we have to make sure to deallocate the buffer) */
	netbuf_delete(inbuf);
}

#define DHCP_TIMEOUT_MS     2000

static void igmp_thread(void const *arg)
{
	for(;;) {
		osDelay(IGMP_TMR_INTERVAL);
		igmp_tmr();
	}
}

/*
static void dhcpc_thread(void const *arg)
{
	err_t err;
	uint32_t mscnt = 0;

	dhcp_set_struct(&g_netif, &st_dhcp);

	err = dhcp_start(&g_netif);
	if(err != ERR_OK) {
	  LWIP_DEBUGF(LWIP_DBG_ON, ("DHCP failed"));
	}

	for(;;) {
		osDelay(DHCP_FINE_TIMER_MSECS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS * 1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
}*/

#if 0
static void httpd_task (void const *arg)
{
	struct netconn *conn, *newconn;
	err_t err, accept_err;
	ip_addr_t ip;

	networkd_start();

	//osThreadCreate(osThread(dhcpc_thread), 0);

	for(;;) {
		//if(gnetif.ip_addr.addr) {
		//	break;
		//}
		networkd_get_ip(&ip);

		if(ip.addr) {
			break;
		}

		osDelay(500);
	}

	mdnsd_start();

	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);

	if (conn!= NULL)
	{
		/* Bind to port 80 (HTTP) with default IP address */
		err = netconn_bind(conn, NULL, 80);

		if (err == ERR_OK)
		{
			/* Put the connection into LISTEN state */
			netconn_listen(conn);

			while(1)
			{
				/* accept any icoming connection */
				accept_err = netconn_accept(conn, &newconn);
				if(accept_err == ERR_OK)
				{
					/* serve connection */
					http_server_serve(newconn);

					/* delete connection */
					netconn_delete(newconn);
				}
			}
		}
	}
}
#endif

static void init_task (void const *arg)
{
	osThreadCreate(osThread(httpd_task), 0);
	osThreadCreate(osThread(led_blink_task), 0);
	osThreadCreate(osThread(gui_task), 0);
}

static void gui_task(void const *arg)
{
	BSP_LCD_Init();
	BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
//	BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS + (BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));
	BSP_LCD_DisplayOn();
	BSP_LCD_SelectLayer(0);
	BSP_LCD_Clear(LCD_COLOR_BLACK);
//    BSP_LCD_SelectLayer(1);
//    BSP_LCD_Clear(LCD_COLOR_BLACK);

	while(1) {
		BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
		BSP_LCD_DisplayStringAtLine(1, "Hello World");
		osDelay(500);
	}
}

void main(void)
{
	MPU_Config();
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	{
		extern void kfw_startup();
		kfw_startup();
	}

	//osKernelInitialize();
	//osThreadCreate(osThread(init_task), 0);
	//osKernelStart();
}

void _init()
{

}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 200 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20010000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}


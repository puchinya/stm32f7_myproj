
#include <lwip/netif.h>

typedef struct {

} ws_context_t;

void ws_start_session(void);

void ws_process_frame(void)
{
	uint8_t d0, d1;
	uint32_t payload_len;

	d0 = ws_read();
	d1 = ws_read();

	payload_len = d1 & 0x7F;

	if(payload_len >= 126) {
		uint32_t pl0 = ws_read();
		uint32_t pl1 = ws_read();

		payload_len = (pl0 << 8) | pl1;

		if(payload_len >= 127) {
			//
		}
	}

}

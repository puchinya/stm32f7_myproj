
typedef void *om2m_pal_ws_conn_t;
typedef void *om2m_pal_ws_msg_t;
typedef int om2m_err_t;

#include <stdint.h>

om2m_err_t om2m_pal_ws_connect(const char *server_url, uint32_t server_url_len,
		om2m_pal_ws_conn_t **conn);
om2m_err_t om2m_pal_ws_disconnect(om2m_pal_ws_conn_t *conn);
om2m_err_t om2m_pal_ws_send(om2m_pal_ws_conn_t *conn, om2m_pal_ws_msg_t *msg);
om2m_err_t om2m_pal_ws_recv(om2m_pal_ws_conn_t *conn, om2m_pal_ws_msg_t **msg);
om2m_err_t om2m_pal_ws_msg_alloc(om2m_pal_ws_conn_t *conn, om2m_pal_ws_msg_t **msg);
om2m_err_t om2m_pal_ws_msg_addref(om2m_pal_ws_msg_t *msg);
om2m_err_t om2m_pal_ws_msg_release(om2m_pal_ws_msg_t *msg);
void *om2m_pal_ws_msg_get_data(om2m_pal_ws_msg_t *msg);
uint32_t om2m_pal_ws_msg_get_data_size(om2m_pal_ws_msg_t *msg);

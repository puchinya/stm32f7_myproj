/*
 * httpd.h
 *
 *  Created on: 2017/09/23
 *      Author: masatakanabeshima
 */

#ifndef HTTPD_H_
#define HTTPD_H_

#include <itron.h>

typedef struct {
	kos_uint_t workers_cnt;

} kos_chttpd_t;

typedef struct httpd_t httpd_t;

kos_er_t kos_cre_httpd(httpd_t *httpd, kos_chttpd_t *chttpd);
kos_er_t kos_del_httpd(httpd_t *httpd);

kos_er_t kos_sta_httpd(httpd_t *httpd);
kos_er_t kos_ter_httpd(httpd_t *httpd);

#endif /* HTTPD_H_ */

/*
 * HttpFacade.h
 *
 *  Created on: Nov 4, 2015
 *      Author: eai
 */

#ifndef HTTPFACADE_H_
#define HTTPFACADE_H_


#define HTTPFAC_OK                   0
#define HTTPFAC_CANNOT_CREATE_FILE  -1
#define HTTPFAC_INVALID_URL         -2
#define HTTPFAC_IOERROR             -3

#include "TCPFacade.h"
#include "URL.h"
#include "HttpProxy.h"


#define HTTP_CLLBCK_ERR_MOVED           (TCP_CLLBCK_ERR_APPLAYER_BASE + 0)
#define HTTP_CLLBCK_ERR_FORBIDDEN       (TCP_CLLBCK_ERR_APPLAYER_BASE + 1)
#define HTTP_CLLBCK_ERR_NOT_FOUND       (TCP_CLLBCK_ERR_APPLAYER_BASE + 2)
#define HTTP_CLLBCK_ERR_PROXY_AUTH      (TCP_CLLBCK_ERR_APPLAYER_BASE + 3)
#define HTTP_CLLBCK_ERR_BAD_GATEWAY     (TCP_CLLBCK_ERR_APPLAYER_BASE + 4)
#define HTTP_CLLBCK_ERR_SERV_UNAVAIL    (TCP_CLLBCK_ERR_APPLAYER_BASE + 5)
#define HTTP_CLLBCK_ERR_VER_NOT_SUPP    (TCP_CLLBCK_ERR_APPLAYER_BASE + 6)
#define HTTP_CLLBCK_ERR_SERVER_ERR      (TCP_CLLBCK_ERR_APPLAYER_BASE + 7)
#define HTTP_CLLBCK_ERR_REQ_ERR         (TCP_CLLBCK_ERR_APPLAYER_BASE + 8)
#define HTTP_CLLBCK_ERR_UNKNOWN         (TCP_CLLBCK_ERR_APPLAYER_BASE + 9)
#define HTTP_CLLBCK_TUNNEL_ERR          (TCP_CLLBCK_ERR_APPLAYER_BASE +10)
#define translate_http_err(e) ( \
			(e == HTTP_CLLBCK_ERR_MOVED)? "Moved": \
			(e == HTTP_CLLBCK_ERR_FORBIDDEN)? "Forbidden": \
			(e == HTTP_CLLBCK_ERR_NOT_FOUND)? "Not Found": \
			(e == HTTP_CLLBCK_ERR_PROXY_AUTH)? "Proxy authentication required": \
			(e == HTTP_CLLBCK_ERR_BAD_GATEWAY)? "Bad Gateway": \
			(e == HTTP_CLLBCK_ERR_SERV_UNAVAIL)? "Service Unavailable": \
			(e == HTTP_CLLBCK_ERR_VER_NOT_SUPP)? "Http Version not supported": \
			(e == HTTP_CLLBCK_ERR_SERVER_ERR)? "Server Error": \
			(e == HTTP_CLLBCK_ERR_REQ_ERR)? "Request error": \
			(e == HTTP_CLLBCK_ERR_UNKNOWN)? "Unknown HTTP error": \
			(e == HTTP_CLLBCK_TUNNEL_ERR)? "HTTP tunnel error": \
			translate_tcp_err(e) \
		)

typedef enum http_connect_type {httpct_direct, httpct_proxy, httpct_tunnel} http_connect_type_t;

class HttpFacade {
public:
	HttpFacade();
	virtual ~HttpFacade();

	static HttpFacade instance;

	HttpProxy* proxy;

	// Downloads the content referenced by the given URL and writes it to the given file.
	// From time to time the data_recv_callback function will be called. If the callback returns
	// something different from TCPP_OK, the process is cancelled.
	// At the end the close_callback function is called.
	// The download function returns a value < 0 if an error occurred with the given error code.
	int download(const char* url, const char* file, data_recv_callback_t data_recv_callback, close_callback_t close_callback, error_callback_t error_callback, http_connect_type_t conn_type);
	int download(URL& url, const char* file, data_recv_callback_t data_recv_callback, close_callback_t close_callback, error_callback_t error_callback, http_connect_type_t conn_type);

	tcp_connect_resp_t tunnel_data_to(HttpProxy* proxy, URL& url, const void* data, uint16_t len, callback_set_t callback_set);
	tcp_connect_resp_t tunnel_data_to(HttpProxy* proxy, URL& url, const char* data, callback_set_t callback_set);
	tcp_connect_resp_t tunnel_data_to(HttpProxy* proxy, URL& url, string& data, callback_set_t callback_set);
};

#endif /* HTTPFACADE_H_ */

/*
 * HttpRequest.h
 *
 *  Created on: Nov 11, 2015
 *      Author: eai
 */

#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include "URL.h"
#include "HttpProxy.h"

class HttpRequest {
public:
	HttpRequest(URL& url);
	HttpRequest(URL& url, HttpProxy* proxy, bool tunnel);
	virtual ~HttpRequest();

	const char* get_request_host() const { return host; };
	uint16_t get_request_port() const { return port; };
	const char* get_request_str() const { return http_request; };
private:
	char* http_request;
	const char* host;
	uint16_t port;
};

#endif /* HTTPREQUEST_H_ */

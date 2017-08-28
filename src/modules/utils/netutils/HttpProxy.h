/*
 * HttpProxy.h
 *
 *  Created on: Nov 13, 2015
 *      Author: eai
 */

#ifndef HTTPPROXY_H_
#define HTTPPROXY_H_

#include <inttypes.h>

// This class represents all the settings needed to connect to an HTTP Proxy.
class HttpProxy {
public:
	HttpProxy(const char* host, uint16_t port);
	HttpProxy(const HttpProxy& proxy);
	virtual ~HttpProxy();
	HttpProxy& operator = (const HttpProxy& proxy);
	const char* get_host() const {return host;};
	uint16_t get_port() const {return port;};
private:
	char* host;
	uint16_t port;
};

#endif /* HTTPPROXY_H_ */

/*
 * HttpProxy.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: eai
 */

#include "HttpProxy.h"

#include <string.h>

HttpProxy::HttpProxy(const char* host, uint16_t port) : port(port) {
	this->host = strdup(host);
}
HttpProxy::HttpProxy(const HttpProxy& proxy) : HttpProxy(proxy.host, proxy.port) {
}

HttpProxy::~HttpProxy() {
	delete [] this->host;
}

HttpProxy& HttpProxy::operator = (const HttpProxy& proxy){
	delete [] this->host;
	this->host = strdup(proxy.host);
	this->port = proxy.port;
	return *this;
}

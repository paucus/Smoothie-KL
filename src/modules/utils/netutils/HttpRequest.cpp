/*
 * HttpRequest.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: eai
 */

#include "HttpRequest.h"
#include "utils.h"
#include <stdio.h>

constexpr char HTTPREQ_NO_PROXY[] = "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n";
constexpr char HTTPREQ_PROXY[] = "GET %s://%s:%d/%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n";
constexpr char HTTPREQ_PROXY_CONNECT[] = "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n";
// WARNING: Use printf_fmt_len with caution. If you are using strange printf features, verify the result is ok.
constexpr unsigned int HTTPREQ_NO_PROXY_LEN = print_fmt_len(HTTPREQ_NO_PROXY);
constexpr unsigned int HTTPREQ_PROXY_LEN = print_fmt_len(HTTPREQ_PROXY);
constexpr unsigned int HTTPREQ_PROXY_CONNECT_LEN = print_fmt_len(HTTPREQ_PROXY_CONNECT);

HttpRequest::HttpRequest(URL& url) : HttpRequest(url, nullptr, false) {
}
HttpRequest::HttpRequest(URL& url, HttpProxy* proxy, bool tunnel) {
	if (proxy) {
		// Proxied communication. Use the Proxy host+port.
		host = proxy->get_host();
		port = proxy->get_port();
		if (!tunnel) {
			http_request = new char[HTTPREQ_PROXY_LEN + strlen(url.get_protocol()) + 2*strlen(url.get_host()) + num_digits(url.get_port()) + strlen(url.get_path())];
			sprintf(http_request, HTTPREQ_PROXY, url.get_protocol(), url.get_host(), url.get_port(), url.get_path(), url.get_host());
		} else {
			http_request = new char[HTTPREQ_PROXY_CONNECT_LEN + 2*strlen(url.get_host()) + 2*num_digits(url.get_port())];
			sprintf(http_request, HTTPREQ_PROXY_CONNECT, url.get_host(), url.get_port(), url.get_host(), url.get_port());
		}
	} else {
		http_request = new char[HTTPREQ_NO_PROXY_LEN + strlen(url.get_path()) + strlen(url.get_host())];
		// Direct communication. Use the URL host+port.
		host = url.get_host();
		port = url.get_port();
		sprintf(http_request, HTTPREQ_NO_PROXY, url.get_path(), url.get_host());
	}
}

HttpRequest::~HttpRequest() {
	if (http_request) {
		delete [] http_request;
	}
}


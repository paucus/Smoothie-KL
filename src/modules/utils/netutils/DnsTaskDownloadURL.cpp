/*
 * DnsTaskDownloadURL.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: eai
 */

#include "DnsTaskDownloadURL.h"
#include <string.h>
#include "TCPFacade.h"

DnsTaskDownloadURL::DnsTaskDownloadURL(URL& url, const char* file, data_recv_callback_t data_recv_callback, close_callback_t close_callback, error_callback_t callback_error, http_connect_type_t conn_type) : url(url), data_recv_callback(data_recv_callback), close_callback(close_callback), callback_error(callback_error), conn_type(conn_type) {
	this->file = new char[strlen(file) + 1];
	strcpy(this->file, file);
}

DnsTaskDownloadURL::~DnsTaskDownloadURL() {
	delete [] file;
}

const char* DnsTaskDownloadURL::get_name_to_lookup() {
	return this->url.get_host();
}

void DnsTaskDownloadURL::proceed(const char* host, uint16_t* ip) {
	if (!ip) {
		// Failed to resolve the host name. Cancel the download process.
		if (this->callback_error) this->callback_error(TCP_CLLBCK_ERR_HOST_UNKNOWN);
	} else {
		HttpFacade::instance.download(this->url, this->file, this->data_recv_callback, this->close_callback, this->callback_error, this->conn_type);
	}
}

void DnsTaskDownloadURL::timeout() {
	this->callback_error(TCP_CLLBCK_ERR_DNS_TIMEOUT);
}

DnsTask* DnsTaskDownloadURL::heap_clone() {
	DnsTaskDownloadURL* t = new DnsTaskDownloadURL(this->url, this->file, this->data_recv_callback, this->close_callback, this->callback_error, this->conn_type);
	t->task_timeout = this->task_timeout;
	return t;
}

/*
 * DnsTaskDownloadURL.h
 *
 *  Created on: Nov 5, 2015
 *      Author: eai
 */

#ifndef DNSTASKDOWNLOADURL_H_
#define DNSTASKDOWNLOADURL_H_

#include "DnsTasks.h"
#include "URL.h"
#include "HttpFacade.h"

class DnsTaskDownloadURL: public DnsTask {
public:
	DnsTaskDownloadURL(URL& url, const char* file, data_recv_callback_t data_recv_callback, close_callback_t close_callback, error_callback_t callback_error, http_connect_type_t conn_type);
	virtual ~DnsTaskDownloadURL();
	const char* get_name_to_lookup();
	void proceed(const char* host, uint16_t* ip);
	void timeout();
	DnsTask* heap_clone();
private:
	URL url;
	char* file;
	data_recv_callback_t data_recv_callback;
	close_callback_t close_callback;
	error_callback_t callback_error;
	http_connect_type_t conn_type;
};

#endif /* DNSTASKDOWNLOADURL_H_ */

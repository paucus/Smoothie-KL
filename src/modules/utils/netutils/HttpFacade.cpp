/*
 * HttpFacade.cpp
 *
 *  Created on: Nov 4, 2015
 *      Author: eai
 */

#include "HttpFacade.h"
#include <stdio.h>
#include "DnsTasks.h"
#include "DnsTaskDownloadURL.h"
#include "utils.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpResponseStream.h"

HttpFacade HttpFacade::instance;

HttpFacade::HttpFacade() : proxy(nullptr) {

}

HttpFacade::~HttpFacade() {
}


int HttpFacade::download(const char* url_addr, const char* file, data_recv_callback_t data_recv_callback, close_callback_t close_callback, error_callback_t error_callback, http_connect_type_t conn_type) {
	URL url(url_addr);
	return HttpFacade::instance.download(url, file, data_recv_callback, close_callback, error_callback, conn_type);
}
int HttpFacade::download(URL& url, const char* file, data_recv_callback_t data_recv_callback, close_callback_t close_callback, error_callback_t callback_error, http_connect_type_t conn_type) {
	if (!url.is_valid()) {
		return HTTPFAC_INVALID_URL;
	}
	FILE* f = fopen(file, "wb");

	if (!f) {
		return HTTPFAC_CANNOT_CREATE_FILE;
	}

	// Create a new response object to be populated later
	HttpResponse* httpResp = new HttpResponse();
	HttpResponseStream* rstream = new HttpResponseStream(httpResp);

	data_recv_callback_t tcp_segm_recv_callback = [f, data_recv_callback, httpResp, rstream, callback_error] (const void* data, uint16_t len) -> tcpprogress_resp_t {
		rstream->parse(data, len);
		if (httpResp->body_segment_len == 0) {
			// Body hasn't been reached yet. Don't write anything yet.
			return TCPP_OK;
		}
		if (httpResp->status.is_set && httpResp->status.val / 100 > 2) { // Only accept 1xx and 2xx status
			// Bad status
			if (httpResp->status.val / 100 == 3) {
				// Moved
				callback_error(HTTP_CLLBCK_ERR_MOVED);
			} else if (httpResp->status.val == 403) {
				// Forbidden
				callback_error(HTTP_CLLBCK_ERR_FORBIDDEN);
			} else if (httpResp->status.val == 404) {
				// Not found
				callback_error(HTTP_CLLBCK_ERR_NOT_FOUND);
			} else if (httpResp->status.val == 407) {
				// Proxy authentication required
				callback_error(HTTP_CLLBCK_ERR_PROXY_AUTH);
			} else if (httpResp->status.val == 502) {
				// Bad Gateway
				callback_error(HTTP_CLLBCK_ERR_BAD_GATEWAY);
			} else if (httpResp->status.val == 503) {
				// Service Unavailable
				callback_error(HTTP_CLLBCK_ERR_SERV_UNAVAIL);
			} else if (httpResp->status.val == 505) {
				// Http Version not supported
				callback_error(HTTP_CLLBCK_ERR_VER_NOT_SUPP);
			} else if (httpResp->status.val / 100 == 5) {
				// Remaining 5xx error
				callback_error(HTTP_CLLBCK_ERR_SERVER_ERR);
			} else if (httpResp->status.val / 100 == 4) {
				// Remaining 4xx error
				callback_error(HTTP_CLLBCK_ERR_REQ_ERR);
			} else {
				// Mhh, something looks wrong
				callback_error(HTTP_CLLBCK_ERR_UNKNOWN);
			}

			return TCPP_ERROR;
		}

		const void* towrite = httpResp->body_segment_ptr;
		int retries = 10;
		int pending_to_write = httpResp->body_segment_len;
		while (pending_to_write > 0 && retries >= 0) {
			int written = fwrite(towrite, 1, pending_to_write, f);
			pending_to_write -= written;
			towrite = &(static_cast<const char*>(towrite)[written]);
			if (pending_to_write > 0) {
				if (written == 0) {
					// Only discount retries when no progress has been performed.
					retries--;
				}
			}
		}
		fflush(f);

		if (retries < 0) {
			// Failed to write file. Cancel!!
			return TCPP_ERROR;
		}

		if (data_recv_callback) {
			return data_recv_callback(data, len);
		} else {
			return TCPP_OK;
		}
	};
	close_callback_t callback_tcp_close = [f, close_callback, httpResp, rstream] () {
		fclose(f);
		delete httpResp;
		delete rstream;
		if (close_callback) {
			close_callback();
		}
	};
	error_callback_t callback_tcp_err = [callback_error] (int err_num) {
		if (callback_error) {
			callback_error(err_num);
		}
	};

	// TODO Add User-agent=Kikai?, Accept
	tcp_connect_resp_t resp;
	if (!proxy || conn_type == httpct_direct) {
		HttpRequest req(url, proxy, false);
		resp = TCPFacade::send_data_to(req.get_request_host(), req.get_request_port(), req.get_request_str(), callback_set_t(tcp_segm_recv_callback, [](char** data, uint16_t* len){}, callback_tcp_close, callback_tcp_err));
	} else {
		// Proxy = null because we are tunnelling. This request is the one to be stablished with
		// the real endpoint, not the tunnel.
		HttpRequest req(url, nullptr, false);
		resp = tunnel_data_to(proxy, url, req.get_request_str(), callback_set_t(tcp_segm_recv_callback, [](char**data,uint16_t*len){}, callback_tcp_close, callback_tcp_err));
	}
	if (resp == TCPC_DNS_RETRY){
		DnsTaskDownloadURL t(url, file, data_recv_callback, close_callback, callback_error, conn_type);
		DnsTasks::instance.append_task(&t);
	} else if (resp != TCPC_OK){
		return HTTPFAC_IOERROR;
	}

	return HTTPFAC_OK;
}

tcp_connect_resp_t HttpFacade::tunnel_data_to(HttpProxy* proxy, URL& url, const void* data, uint16_t len, callback_set_t callback_set) {
	HttpRequest req(url, proxy, true);

	// Create a new response object to be populated later. This is the tunnel envelope response, not the tunnelled connection.
	HttpResponse* httpResp = new HttpResponse();
	HttpResponseStream* rstream = new HttpResponseStream(httpResp);
	int* tunnel_conn_status = new int;
	*tunnel_conn_status = 0; // 0 = still hasn't received answer, 1 = answer received, waiting to send reply, 2 = reply sent/nothing else to do

	data_recv_callback_t tunnel_segm_recv_callback = [data_recv_callback=callback_set.data_recv_callback, httpResp, rstream, error_callback=callback_set.error_callback, tunnel_conn_status] (const void* data, uint16_t len) -> tcpprogress_resp_t {

		rstream->parse(data, len);

		if (httpResp->body_segment_len > 0) {
			// Pass the data received to the tunnelled connection
			tcpprogress_resp_t resp = data_recv_callback(httpResp->body_segment_ptr, httpResp->body_segment_len);
			if (resp != TCPP_OK) {
				// forward any non OK message (TCPP_CANCEL to cancel, TCPP_RESP to answer from the
				// tunneled input, and error to forward an error notification)
				return resp;
			}
		}

		// Tunnels might have no data until we send a reply with the corresponding GET, so, queue a response
		if (httpResp->status.is_set){
			if (httpResp->status.val / 100 != 2) { // Only accept 200 status
				// Tunnel error
				error_callback(HTTP_CLLBCK_TUNNEL_ERR);

				return TCPP_ERROR;
			} else if (*tunnel_conn_status == 0) { // status=200 + still hasn't received answer
				// Send the answer
				*tunnel_conn_status = 1;
				return TCPP_RESP;
			}
		}

		return TCPP_OK;
	};

	answer_callback_t answer_callback = [data, len, tunnel_conn_status] (char** data_p, uint16_t* len_p) {
		// Copy answer to pointer
		*tunnel_conn_status = 2;
		*data_p = new char[len];
		*len_p = len;
		memcpy(*data_p, data, len);
	};

	close_callback_t callback_close_wrap = [close_callback=callback_set.close_callback, httpResp, rstream, tunnel_conn_status]() {
		delete httpResp;
		delete rstream;
		delete tunnel_conn_status;
		if (close_callback) {
			close_callback();
		}
	};

	// Temporarily join the tunnel and tunnelled request
	tcp_connect_resp_t resp = TCPFacade::send_data_to(req.get_request_host(), req.get_request_port(), req.get_request_str(), callback_set_t(tunnel_segm_recv_callback, answer_callback, callback_close_wrap, callback_set.error_callback));

	return resp;
}

tcp_connect_resp_t HttpFacade::tunnel_data_to(HttpProxy* proxy, URL& url, const char* data, callback_set_t callback_set) {
	return this->tunnel_data_to(proxy, url, static_cast<const void*>(data), strlen(data), callback_set);
}
tcp_connect_resp_t HttpFacade::tunnel_data_to(HttpProxy* proxy, URL& url, string& data, callback_set_t callback_set) {
	return this->tunnel_data_to(proxy, url, static_cast<const void*>(data.c_str()), data.length(), callback_set);
}


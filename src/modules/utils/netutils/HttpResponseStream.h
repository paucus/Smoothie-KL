/*
 * HttpResponseStream.h
 *
 *  Created on: Nov 11, 2015
 *      Author: eai
 */

#ifndef HTTPRESPONSESTREAM_H_
#define HTTPRESPONSESTREAM_H_

#include "HttpResponse.h"
#include <inttypes.h>

#define HRS_HTTP_STATUS "HTTP/1.1 "
#define HRS_CONTENT_LENGTH "Content-Length:"
//#define HRS_SET_COOKIE "Set-Cookie:"


// We don't need the "real" max length, just up to the length we need.
// We only want the status number, not the text after it.
#define HRS_HTTP_STATUS_MAX_LEN sizeof("HTTP/1.1 200 ")
// We can't store more than 4GB, so, ignore the rest
#define HRS_CONTENT_LENGTH_MAX_LEN sizeof("Content-Length: 4294967295")
// The cookie string will be parsed differently.
//#define HRS_SET_COOKIE_MAX_LEN sizeof("Set-Cookie: ")

enum hts_state {hrs_unknown, hrs_http_body, hrs_ignore_line, hrs_http_status, hrs_content_length};
//, hrs_set_cookie

#define max(a,b) (a>b?a:b)
#define max3(a,b,c) (max(max(a,b), c))
// The +1 is for the ending NULL char
#define HRS_MAX_HEADER_LEN (max(sizeof(HRS_HTTP_STATUS), sizeof(HRS_CONTENT_LENGTH)))
//, sizeof(HRS_SET_COOKIE)
#define HRS_MAX_BUFF_LEN (max(HRS_HTTP_STATUS_MAX_LEN, HRS_CONTENT_LENGTH_MAX_LEN))
//, HRS_SET_COOKIE_MAX_LEN

// This class is responsible for populating a response object
class HttpResponseStream {
public:
	HttpResponseStream(HttpResponse* resp);
	virtual ~HttpResponseStream();
	void parse(const void* data, uint16_t len);
private:
	HttpResponse* resp;
	hts_state state;

	void process_state_unknown(const char* data, unsigned int & i, uint16_t len);
	void process_state_ignore_line(const char* data, unsigned int & i, uint16_t len);
	void process_state_http_status(const char* data, unsigned int & i, uint16_t len);
	void process_state_content_length(const char* data, unsigned int & i, uint16_t len);
//	void process_state_set_cookie(const char* data, unsigned int & i, uint16_t len);

	// status while processing
	char current_line[HRS_MAX_BUFF_LEN];
	char current_line_len;

};

#endif /* HTTPRESPONSESTREAM_H_ */

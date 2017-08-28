/*
 * HttpResponse.h
 *
 *  Created on: Nov 11, 2015
 *      Author: eai
 */

#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <inttypes.h>
using namespace std;
#include <string>
#include "opt_attr.h"

class HttpResponse {
public:
	HttpResponse();
	virtual ~HttpResponse();

	struct opt_attr<uint16_t> status;
	struct opt_attr<unsigned long> content_length;
//	struct opt_attr<string> cookie;
	const void* body_segment_ptr;
	uint16_t body_segment_len;

private:
};

#endif /* HTTPRESPONSE_H_ */

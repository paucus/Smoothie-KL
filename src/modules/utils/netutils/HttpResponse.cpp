/*
 * HttpResponse.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: eai
 */

#include "HttpResponse.h"

HttpResponse::HttpResponse() : body_segment_ptr(nullptr), body_segment_len(0) {
}

HttpResponse::~HttpResponse() {
}


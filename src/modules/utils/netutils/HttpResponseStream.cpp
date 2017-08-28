/*
 * HttpResponseStream.cpp
 *
 *  Created on: Nov 11, 2015
 *      Author: eai
 */

#include "HttpResponseStream.h"
#include <string.h>

#include <cstdlib>

HttpResponseStream::HttpResponseStream(HttpResponse* resp) : resp(resp), state(hrs_unknown), current_line_len(0) {

}

HttpResponseStream::~HttpResponseStream() {
}

// Comparison is CASE INSENSITIVE, and we must remember to compare only up to the length of the
// header (sizeof(header_const) - 1). That's why I created this function, to avoid forgetting it.
// (header_const MUST be a constant to use sizeof)
#define is_header(header_const, line, line_len) (line_len == sizeof(header_const) && strncasecmp(header_const, line, sizeof(header_const) - 1) == 0)

// This function skips any '\r' and returns true if and only if the end of the segment was reached
static bool skip_cr_and_check_eof(char& c, const char* data, unsigned int & i, uint16_t len) {
	while (c == '\r') {
		i++;
		if (i < len) return true;
		c = data[i];
	}
	return false;
}
// Skips N chars where N is the length of the constant string text_to_skip (MUST be a constant to use sizeof)
#define skip_const_text(text,text_to_skip) (&(text[sizeof(text_to_skip)-1]))

void HttpResponseStream::process_state_unknown(const char* data, unsigned int & i, uint16_t len) {
	// When we enter this function we know it is either because a new segment arrived and we must
	// continue what was being done, or because the state was just changed. If we have just changed
	// the state, then we must check if the header has finished before starting.
	char c = data[i];
	if (current_line_len == 0) {
		// Beginning of the header. Check if it is a newline.
		if (skip_cr_and_check_eof(c,data,i,len)) return;

		if (c=='\n'){
			// New line char!! We have found the end of the header
			state = hrs_http_body;
			i++; // Skip the '\n' char so that i points to the beginning of the body

			// Update response body ptr
			resp->body_segment_ptr = &(data[i]);
			resp->body_segment_len = len - i;

			return;
		}
	}

	// From this point on, we know that we won't be at the beginning of the line, so the state
	// won't change to hrs_http_body.

	do {
		// Not a newline. Append it to the buffer.
		current_line[current_line_len] = c;
		current_line_len++;
		i++;

		// We can verify if it is the HTTP status field
		if (is_header(HRS_HTTP_STATUS, current_line, current_line_len)) {
			state = hrs_http_status;
			return;
		}
		if (is_header(HRS_CONTENT_LENGTH, current_line, current_line_len)) {
			state = hrs_content_length;
			return;
		}
//		if (is_header(HRS_SET_COOKIE, current_line, current_line_len)) {
//			state = hrs_set_cookie;
//			return;
//		}
		if (current_line_len > HRS_MAX_HEADER_LEN) {
			// So far we haven't found any header that we could be interested in, so, ignore the rest of the line.
			state = hrs_ignore_line;
			return;
		}

		c = data[i];

	} while (c != '\n' && i < len);

	if (c == '\n') {
		// New line char!! We have found the end of this header field.
		// Reset the current_line+current_line_len and re-process it as a new field.
		current_line_len = 0;
		i++; // Skip the '\n' char so that i points to the beginning of the next header field
		return;
	}

	// Otherwise we have reached the end of the segment. We mustn't do anything so that we proceed later.
}

void HttpResponseStream::process_state_ignore_line(const char* data, unsigned int & i, uint16_t len) {
	// proceed until we find a new line
	while(i < len && data[i] != '\n'){
		i++;
	}
	if (data[i] == '\n') {
		state = hrs_unknown; // Start processing the new header
		current_line_len = 0;
		i++; // Skip the '\n' char so that i points to the beginning of the next header field
	}
}

void HttpResponseStream::process_state_http_status(const char* data, unsigned int & i, uint16_t len) {
	// At this point we are at the middle of the line. Keep copying until we reach the end of the
	// line or the max length we need.
	while (i < len && current_line_len < HRS_HTTP_STATUS_MAX_LEN && data[i] != '\n') {
		current_line[current_line_len] = data[i];
		current_line_len++;
		i++;
	}
	if (!(i < len)){
		// The segment ended here, but there might be data to process
		return;
	}

	// parse the status code.
	current_line[current_line_len] = '\0'; // Append a NULL char to end the string
	resp->status = strtoul(skip_const_text(current_line, HRS_HTTP_STATUS), NULL, 10);

	// Skip the rest of the string
	state = hrs_ignore_line;
}

void HttpResponseStream::process_state_content_length(const char* data, unsigned int & i, uint16_t len) {
	// At this point we are at the middle of the line. Keep copying until we reach the end of the
	// line or the max length we need.
	while (i < len && current_line_len < HRS_CONTENT_LENGTH_MAX_LEN && (data[i] != '\n' && data[i] != '\r')) {
		current_line[current_line_len] = data[i];
		current_line_len++;
		i++;
	}
	if (!(i < len)){
		// The segment ended here, but there might be data to process
		return;
	}

	// parse the content length.
	current_line[current_line_len] = '\0'; // Append a NULL char to end the string
	resp->content_length = strtoul(skip_const_text(current_line, HRS_CONTENT_LENGTH), NULL, 10);

	// Skip the rest of the string
	state = hrs_ignore_line;
}

//void HttpResponseStream::process_state_set_cookie(const char* data, unsigned int & i, uint16_t len) {
//	// WARNING: NOT IMPLEMENTED YET. This is a placeholder.
//
//	// At this point we are at the middle of the line. Keep copying until we reach the end of the
//	// line or the max length we need.
//	while (i < len && current_line_len < HRS_CONTENT_LENGTH_MAX_LEN && (data[i] != '\n' && data[i] != '\r')) {
//		current_line[current_line_len] = data[i];
//		current_line_len++;
//		i++;
//	}
//	if (!(i < len)){
//		// The segment ended here, but there might be data to process
//		return;
//	}
//
//	// Skip the rest of the string
//	state = hrs_ignore_line;
//}

void HttpResponseStream::parse(const void* data, uint16_t len) {
	if (state == hrs_http_body){
		// Update body ptr in response. Nothing else to do.
		resp->body_segment_ptr = data;
		resp->body_segment_len = len;
		return;
	}

	unsigned int i = 0;
	const char* data_str = static_cast<const char*>(data);
	while (i < len && state != hrs_http_body) {
		if (state == hrs_unknown) {
			process_state_unknown(data_str, i, len);
		} else if (state == hrs_ignore_line) {
			process_state_ignore_line(data_str, i, len);
		} else if (state == hrs_http_status) {
			process_state_http_status(data_str, i, len);
		} else if (state == hrs_content_length) {
			process_state_content_length(data_str, i, len);
//		} else if (state == hrs_set_cookie) {
//			process_state_set_cookie(data_str, i, len);
		}
	}

}


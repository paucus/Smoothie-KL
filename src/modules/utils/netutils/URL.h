/*
 * URL.h
 *
 *  Created on: Nov 4, 2015
 *      Author: eai
 */

#ifndef URL_H_
#define URL_H_

#include <cstdlib>
#include <string.h>
#include <inttypes.h>

// This class is a very simple implementation of an URL.
class URL {
public:
	URL(const URL& url);
	URL(const char* url);
	virtual ~URL();

	const char* get_host() const { return host; };
	uint16_t get_port() const { return port; };
	const char* get_path() const { return path; };
	const char* get_protocol() const { return protocol; };
	bool is_valid() const {return addr_buff != NULL;};

private:
	char* host;
	uint16_t port;
	char* path;
	char* protocol;
	// Internal buffer.
	char* addr_buff;
	size_t len;
};

#endif /* URL_H_ */

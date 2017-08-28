/*
 * URL.cpp
 *
 *  Created on: Nov 4, 2015
 *      Author: eai
 */

#include "URL.h"

static uint16_t get_port_from_protocol(const char* protocol) {
	if (strcmp("http", protocol) == 0) {
		return 80;
	} else if (strcmp("https", protocol) == 0) {
		return 443;
	} else if (strcmp("smtp", protocol) == 0) {
		return 25;
	} else {
		// default to 80
		return 80;
	}
}
#include <stdio.h>
URL::URL(const URL& url) {
	if (!url.addr_buff) {
		// Invalid URL
		this->addr_buff = NULL;
		return;
	}
	this->len = url.len;
	this->addr_buff = new char[len+1];
	memcpy(this->addr_buff, url.addr_buff, len+1);
	this->port = url.port;
	this->host = this->addr_buff + (url.host - url.addr_buff);
	this->path = this->addr_buff + (url.path - url.addr_buff);
	this->protocol = this->addr_buff + (url.protocol - url.addr_buff);
}
URL::URL(const char* url) {
	if (url == NULL) {
		// invalid URL
		addr_buff = NULL;
		return;
	}
	len = strlen(url);
//	if (len > MAX_LEN) {
//		// invalid URL
//		addr_buff = NULL;
//		return;
//	}
	if (strchr(url, ' ')) {
		// Common mistake: using white spaces. No space char is allowed.
		addr_buff = NULL;
		return;
	}
	addr_buff = strdup(url);
	protocol = addr_buff;
	host = strchr(protocol, ':');
	if (!host || host[1] != '/' || host[2] != '/' || !(host[3])) { // check ://...
		// invalid URL
		delete [] addr_buff;
		addr_buff = NULL;
		return;
	}
	// This is a temporary string. Set '\0' to finish the protocol string, and skip ://.
	host[0] = '\0';
	host = &(host[3]); // skip ://

	path = strpbrk(host, ":/");
	if (!path) {
		// This means that no port, path or parameters are specified
		path = &(host[strlen(host)]);
		// port number will be specified in the next lines because path[0] != ':'
	}
	if (path[0] == ':') {
		// port given
		port = atoi(&(path[1]));
		path[0] = '\0'; // to end the host string
		path = strchr(&(path[1]), '/');

		if (!path) {
			// No '/' found. This means that no path or parameters are specified. Go to the end.
			path = &(host[strlen(host)]);
		} else {
			// We know we have found '/', so, move one step forward. We don't need to
			// set null char to end a string because the last value was the port number.
			path++;
		}
	} else {
		// no port given. Deduce from known ports.
		port = get_port_from_protocol(protocol);
		// Here we can have path or nothing
		if (path[0] == '/') {
			// We have a path here. Set null char to end the host string and move one step forward.
			path[0] = '\0';
			path++;
		}
	}
}

URL::~URL() {
	if (addr_buff) {
		delete [] addr_buff;
	}
}


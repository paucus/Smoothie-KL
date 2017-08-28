/*
 * DnsTaskNsLookup.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: eai
 */

#include "DnsTaskNsLookup.h"
#include "uip.h"

DnsTaskNsLookup::DnsTaskNsLookup(const char* host, StreamOutput* stream) : stream(stream) {
	this->host = strdup(host);
}

DnsTaskNsLookup::~DnsTaskNsLookup() {
	delete [] this->host;
}

void DnsTaskNsLookup::proceed(const char* host, uint16_t* ip) {
	if (ip) {
		stream->printf("%s: %d.%d.%d.%d\n", host, uip_ipaddr1(ip), uip_ipaddr2(ip), uip_ipaddr3(ip), uip_ipaddr4(ip));
	} else {
		// mhh, strange
		stream->printf("An error occurred while retrieving the IP address of host %s\n", host);
	}
}

void DnsTaskNsLookup::timeout() {
	stream->printf("DNS Timeout when retrieving the IP address of host %s\n", host);
}

DnsTask* DnsTaskNsLookup::heap_clone() {
	DnsTaskNsLookup* t = new DnsTaskNsLookup(host, stream);
	t->task_timeout = this->task_timeout;
	return t;
}

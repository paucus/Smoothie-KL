/*
 * DnsTaskNsLookup.h
 *
 *  Created on: Nov 5, 2015
 *      Author: eai
 */

#ifndef DNSTASKNSLOOKUP_H_
#define DNSTASKNSLOOKUP_H_

#include "DnsTasks.h"
#include "StreamOutput.h"

class DnsTaskNsLookup : public DnsTask {
public:
	DnsTaskNsLookup(const char* host, StreamOutput* stream);
	virtual ~DnsTaskNsLookup();
	const char* get_name_to_lookup() { return host; };
	void proceed(const char* host, uint16_t* ip);
	void timeout();
	DnsTask* heap_clone();
private:
	StreamOutput* stream;
	char* host;
};

#endif /* DNSTASKNSLOOKUP_H_ */

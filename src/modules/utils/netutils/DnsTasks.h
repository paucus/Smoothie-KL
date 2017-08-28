/*
 * DnsTasks.h
 *
 *  Created on: Nov 5, 2015
 *      Author: eai
 */

#ifndef DNSTASKS_H_
#define DNSTASKS_H_

#include <forward_list>
#include <inttypes.h>
#include "Uptime.h"

#define DNSTASK_TIMEOUT_SECS 5

// This class represents a job that is pending due to a DNS resolution in progress
class DnsTask {
public:
	DnsTask() {task_timeout = uptime() + DNSTASK_TIMEOUT_SECS;};
	virtual ~DnsTask() {};
	virtual const char* get_name_to_lookup() = 0;
	virtual void proceed(const char* host, uint16_t* ip) = 0;
	virtual void timeout() = 0;
	virtual DnsTask* heap_clone() = 0;
	time_t task_timeout;
};

class DnsTasks {
public:
	DnsTasks();
	virtual ~DnsTasks();

	static DnsTasks instance;

	void append_task(DnsTask* t);
	void proceed_tasks_for_host(const char* host, uint16_t* ip);
	void check_tasks_timeout();

	std::forward_list<DnsTask*> v;
};

#endif /* DNSTASKS_H_ */

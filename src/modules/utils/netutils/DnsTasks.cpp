/*
 * DnsTasks.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: eai
 */

#include "DnsTasks.h"
#include <string.h>

DnsTasks DnsTasks::instance;

DnsTasks::DnsTasks() {
}

DnsTasks::~DnsTasks() {
}

void DnsTasks::append_task(DnsTask* t) {
	DnsTask* ht = t->heap_clone();
	v.push_front(ht);
}

void DnsTasks::proceed_tasks_for_host(const char* host, uint16_t* ip) {
	v.remove_if([host, ip](DnsTask*& t) -> bool {
		bool host_matches = strcmp(t->get_name_to_lookup(), host) == 0;
		if (host_matches) {
			// This task must proceed
			t->proceed(host, ip);
			// As no one will use this element later, delete it.
			delete t;
		}
		return host_matches;
	});
}

void DnsTasks::check_tasks_timeout() {
	time_t now = uptime();
	v.remove_if([now](DnsTask*& t) -> bool {
		if (t->task_timeout < now) {
			// This task must cancelled
			t->timeout();
			// As no one will use this element later, delete it.
			delete t;
			return true;
		} else {
			return false;
		}
	});
}

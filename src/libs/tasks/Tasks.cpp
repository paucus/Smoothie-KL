/*
 * Tasks.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: eai
 */

#include "Tasks.h"

Tasks* Tasks::instance = nullptr;

Tasks::Tasks() {
	Tasks::instance = this;
}

Tasks::~Tasks() {
}

void Tasks::schedule(CancellableTask* t) {
	tasks.push_front(t);
}
void Tasks::process_tasks() {
	tasks.remove_if([](CancellableTask*& t) -> bool {
		return !t->process();
	});
}
void Tasks::remove_task(CancellableTask* t) {
	tasks.remove(t);
}

void Tasks::on_module_loaded() {
	this->register_for_event(ON_IDLE);
}
void Tasks::on_idle(void *) {
	this->process_tasks();
}

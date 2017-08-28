/*
 * CancellableTask.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: eai
 */

#include "CancellableTask.h"
#include "Tasks.h"

CancellableTask::CancellableTask() {
}

CancellableTask::~CancellableTask() {
}

void CancellableTask::cancel() {
	this->process_cancel();
	Tasks::instance->remove_task(this);
	delete this;
}



/*
 * Tasks.h
 *
 *  Created on: Dec 22, 2015
 *      Author: eai
 */

#ifndef TASKS_H_
#define TASKS_H_

#include <forward_list>
#include "Module.h"
#include "CancellableTask.h"

class Tasks : public Module {
public:
	Tasks();
	virtual ~Tasks();
	static Tasks* instance;

	void on_module_loaded();
	void on_idle(void *);

	// All new tasks expecting to be executed must be scheduled
	void schedule(CancellableTask* t);
	void process_tasks();
	void remove_task(CancellableTask* t);

private:
	std::forward_list<CancellableTask*> tasks;
};

#endif /* TASKS_H_ */

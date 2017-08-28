/*
 * CancellableTask.h
 *
 *  Created on: Dec 22, 2015
 *      Author: eai
 */

#ifndef CANCELLABLETASK_H_
#define CANCELLABLETASK_H_

class CancellableTask {
public:
	CancellableTask();
	virtual ~CancellableTask();
	void cancel();

	// Functions to override
	// Processes part of the task. If the task is finished and must be removed, it returns false.
	virtual bool process() = 0;
protected:
	virtual void process_cancel() = 0;
};

#endif /* CANCELLABLETASK_H_ */

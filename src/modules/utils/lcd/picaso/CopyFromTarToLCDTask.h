/*
 * CopyFromTarToLCDTask.h
 *
 *  Created on: Dec 22, 2015
 *      Author: eai
 */

#ifndef COPYFROMTARTOLCDTASK_H_
#define COPYFROMTARTOLCDTASK_H_

#include "kltar.h"
#include "CancellableTask.h"
#include "smoothie_arduino.h"
#include <functional>


class CopyFromTarToLCDTask: public CancellableTask {
public:
	CopyFromTarToLCDTask(kltar_t* t, word display_filehandle, std::function<void()> on_finish_callback, std::function<void()> cancel_callback);
	virtual ~CopyFromTarToLCDTask();

	bool process();
protected:
	void process_cancel();

private:
	kltar_t* t;
	word display_filehandle;
	std::function<void()> on_finish_callback;
	std::function<void()> cancel_callback;
};

#endif /* COPYFROMTARTOLCDTASK_H_ */

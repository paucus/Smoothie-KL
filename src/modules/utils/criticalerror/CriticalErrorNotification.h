/*
 * CriticalErrorNotification.h
 *
 *  Created on: Apr 30, 2015
 *      Author: eai
 */

#ifndef CRITICALERRORNOTIFICATION_H_
#define CRITICALERRORNOTIFICATION_H_

#include "Module.h"

class CriticalErrorNotification: public Module {
public:
	CriticalErrorNotification(const char* title, const char* msg);
	virtual ~CriticalErrorNotification();

	virtual void on_module_loaded();
	virtual void on_second_tick(void*);

	static CriticalErrorNotification* instance_missing_sd_card_error();
	static CriticalErrorNotification* instance_invalid_dimensions_error();
private:
	const char* title;
	const char* msg;
};

#endif /* CRITICALERRORNOTIFICATION_H_ */

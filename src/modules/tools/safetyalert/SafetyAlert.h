/*
 * SafetyAlert.h
 *
 *  Created on: Mar 4, 2016
 *      Author: eai
 */

#ifndef SAFETYALERT_H_
#define SAFETYALERT_H_

#include "Module.h"

class SafetyAlert : public Module {
public:
	SafetyAlert();
	virtual ~SafetyAlert();

	void on_module_loaded();
	void on_second_tick(void *);
	void on_idle(void *);
private:
	bool hotend_is_heating;
	float last_hotend_measure;
	bool get_measure;
};

#endif /* SAFETYALERT_H_ */

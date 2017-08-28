/*
 * EmergencyStop.h
 *
 *  Created on: Apr 24, 2015
 *      Author: idlt
 */

#ifndef EMERGENCYSTOP_H_
#define EMERGENCYSTOP_H_

#include "Kernel.h"
#include "Module.h"

class EmergencyStop: public Module {
public:
	EmergencyStop();
	virtual ~EmergencyStop();

	void on_module_loaded();
	void on_gcode_received(void* obj);
};

#endif /* EMERGENCYSTOP_H_ */

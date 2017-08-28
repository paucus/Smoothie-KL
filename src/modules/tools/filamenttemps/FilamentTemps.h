/*
 * FilamentTemps.h
 *
 *  Created on: Jul 15, 2015
 *      Author: idlt
 */

#ifndef FILAMENTTEMPS_H_
#define FILAMENTTEMPS_H_

#include "Module.h"

extern float filament_temperatures[4][2];

#define PLA			0
#define ABS			1
#define NYLON		2
#define FLEXIBLE	3

class FilamentTemps: public Module {

public:
	FilamentTemps();
	virtual ~FilamentTemps();

	virtual void on_module_loaded();
	virtual void on_gcode_received(void* arg);

};



#endif /* FILAMENTTEMPS_H_ */

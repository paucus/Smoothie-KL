/*
 * EmergencyStop.cpp
 *
 *  Created on: Apr 24, 2015
 *      Author: idlt
 */

#include "EmergencyStop.h"
#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
#include "Gcode.h"
#include "utils.h"

#define emergency_stop_enable_checksum		CHECKSUM("emergency_stop_enable")

EmergencyStop::EmergencyStop() {

}

EmergencyStop::~EmergencyStop() {
}

void EmergencyStop::on_module_loaded() {
	if (!THEKERNEL->config->value(emergency_stop_enable_checksum)->by_default(true)->as_bool()){
		return;
	}

	this->register_for_event(ON_GCODE_RECEIVED);
}

void EmergencyStop::on_gcode_received(void* obj) {
	Gcode* gcode = static_cast<Gcode*>(obj);
	if (gcode->has_m && gcode->m == 112){
		system_reset(false);
	}
}

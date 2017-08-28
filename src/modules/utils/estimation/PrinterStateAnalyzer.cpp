/*
 * PrinterStateAnalyzer.cpp
 *
 *  Created on: Jul 17, 2015
 *      Author: eai
 */

#include "PrinterStateAnalyzer.h"

#include "Kernel.h"
#include "Config.h"
#include "ConfigValue.h"
#include "utils.h"
#include "checksumm.h"
#include "string.h"
#include "stdlib.h"
#include "PublicData.h"
#include "EndstopsPublicAccess.h"

typedef struct {
	float homing_position[3];
} time_array_config_t;
static time_array_config_t get_config_homing_position() {
	time_array_config_t conf;
	void* result;
	if (!PublicData::get_value(endstops_checksum, homing_position_checksum, &result)) {
		conf.homing_position[X_AXIS] = 0;
		conf.homing_position[Y_AXIS] = 0;
		conf.homing_position[Z_AXIS] = 0;
		return conf;
	}
	copy_vec_x_to_z(conf.homing_position, ((float*)result));
	return conf;
}


PrinterStateAnalyzer::PrinterStateAnalyzer() : PrinterStateAnalyzer(get_config_homing_position().homing_position) {

}

PrinterStateAnalyzer::PrinterStateAnalyzer(const float* homing_position) {
	memcpy(this->homing_position, homing_position, sizeof(this->homing_position));
	absolute = true;
	ext_absolute = true;
}

PrinterStateAnalyzer::~PrinterStateAnalyzer() {
}


float PrinterStateAnalyzer::process_gcode(Gcode* gcode){
	if (gcode->g == 4) {
		// trivial case. A dwell takes exactly the amount of time it says it takes
		if (gcode->has_letter('P')) {
			return gcode->get_value('P') / 1000.0;
		} else if (gcode->has_letter('S')) {
			return gcode->get_value('S');
		}

	} else if (gcode->g == 0 || gcode->g == 1) {
		if (gcode->has_letter('F')){
			last_axis_value[F_AXIS] = gcode->get_value('F');
		}

		// calculate new position
		float new_pos[4];
		copy_vec_x_to_e(new_pos, last_axis_value);

		for (int c = X_AXIS; c <= Z_AXIS; c++) {
			if (gcode->has_letter('X'+c)) {
				if (absolute) {
					new_pos[c] = gcode->get_value('X'+c);
				}else{
					new_pos[c] += gcode->get_value('X'+c);
				}
			}
		}
		if (gcode->has_letter('E')) {
			if (ext_absolute) {
				new_pos[E_AXIS] = gcode->get_value('E');
			}else{
				new_pos[E_AXIS] += gcode->get_value('E');
			}
		}

		// estimate time
		float distance = vec_distance(new_pos, last_axis_value);
		float gcode_time = 0.0;
		if (last_axis_value[F_AXIS] > 0.0)
			gcode_time = distance / last_axis_value[F_AXIS] * 60.0;	// as F is in mm/min, we must adapt to seconds

		// update last known position
		copy_vec_x_to_e(last_axis_value, new_pos);

		return gcode_time;

	} else if (gcode->g == 82 || gcode->g == 83) {
		this->ext_absolute = (gcode->g == 82);
	} else if (gcode->g == 90 || gcode->g == 91) {
		this->absolute = (gcode->g == 90);
		this->ext_absolute = absolute;	// also sets the absolute extruder mode
	} else if (gcode->g == 92) {
		// G92 takes 0 seconds, but we must take note of the new coordinate position if changed
		if (gcode->has_letter('E'))
			last_axis_value[E_AXIS] = gcode->get_value('E');
		for (int c = X_AXIS; c <= Z_AXIS; c++) {
			if (gcode->has_letter('X' + c))
				last_axis_value[c] = gcode->get_value('X' + c);
		}
	} else if (gcode->g == 28) {
		// Ignore the time a home takes as it's unpredictable and it won't be counted for the estimation
		// We only need to update our position in space
		bool homed_any_axis_in_particular = gcode->has_letter('X') && gcode->has_letter('Y') && gcode->has_letter('Z');

		if (homed_any_axis_in_particular || gcode->has_letter('Z')) {
			// Home all or Z ends in 0,0,0,
			for (int c = X_AXIS; c <= Z_AXIS; c++) {
				last_axis_value[c] = 0;
			}
		} else {
			for (int c = X_AXIS; c <= Z_AXIS; c++) {
				bool homed_axis = gcode->has_letter(c + 'X') || !homed_any_axis_in_particular;
				if (homed_axis)
					last_axis_value[c] = homing_position[c];
			}
		}
	}

	return 0;
}

/*
 * Simplify3DDialect.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#include "Simplify3DDialect.h"
#include <string.h>
#include <stdio.h>
#include <cmath>

#define LAYER_HEIGHT_STRING ";   layerHeight,"
#define TEMPERATURE_SETPOINT_TEMPERATURES_STRING ";   temperatureSetpointTemperatures,"
#define TEMPERATURE_HEATED_BED_STRING ";   temperatureHeatedBed,"
#define FILAMENT_LENGTH_STRING ";   Filament length: "
#define PLASTIC_WEIGHT_STRING ";   Plastic weight: "
#define PLASTIC_VOLUME_STRING ";   Plastic volume: "
#define BUILD_TIME_STRING ";   Build time: "

#define LAYER_HEIGHT_SCANF ";   layerHeight,%f"
#define FILAMENT_LENGTH_SCANF ";   Filament length: %f mm ("
#define PLASTIC_WEIGHT_SCANF ";   Plastic weight: %f g ("
#define PLASTIC_VOLUME_SCANF ";   Plastic volume: %f"
#define BUILD_TIME_SCANF ";   Build time: %d hours %f"
#define TEMPERATURE_SETPOINT_TEMPERATURES_SCANF ";   temperatureSetpointTemperatures,%f,%f,%f"
#define TEMPERATURE_HEATED_BED_SCANF ";   temperatureHeatedBed,%d,%d,%d"

Simplify3DDialect::Simplify3DDialect() : t1(0), t2(0), t3(0), b1(0), b2(0), b3(0), temps_read(0), hb_read(0) {
}

Simplify3DDialect::~Simplify3DDialect() {
}

void Simplify3DDialect::parse_line(SlicingInformation* info, const char* line) {
	if (strncmp(line, LAYER_HEIGHT_STRING, strlen(LAYER_HEIGHT_STRING)) == 0) {
		float lh = 0.0;
		sscanf(line, LAYER_HEIGHT_SCANF, &lh);
		// Express the value in microns
		info->layer_height = (int)roundf(lh*1000.0);
	} else if (strncmp(line, FILAMENT_LENGTH_STRING, strlen(FILAMENT_LENGTH_STRING)) == 0) {
		float flength = 0.0;
		sscanf(line, FILAMENT_LENGTH_SCANF, &flength);
		// Express the value in meters
		info->filament_used_meters = flength/1000.0;
	} else if (strncmp(line, PLASTIC_WEIGHT_STRING, strlen(PLASTIC_WEIGHT_STRING)) == 0) {
		float plastic_weight = 0.0;
		sscanf(line, PLASTIC_WEIGHT_SCANF, &plastic_weight);
		// The value is already in grams
		info->filament_used_grams = plastic_weight;
	} else if (strncmp(line, PLASTIC_VOLUME_STRING, strlen(PLASTIC_VOLUME_STRING)) == 0) {
		float plastic_volume = 0.0;
		sscanf(line, PLASTIC_VOLUME_SCANF, &plastic_volume);
		// The value is in mm^3, change it to cm^3
		info->filament_used_vol_cm3 = plastic_volume/1000.0;
	} else if (strncmp(line, BUILD_TIME_STRING, strlen(BUILD_TIME_STRING)) == 0) {
		int hours = 0;
		float mins = 0;
		sscanf(line, BUILD_TIME_SCANF, &hours, &mins);
		info->print_time = hours*3600+mins*60;
	} else if (strncmp(line, TEMPERATURE_SETPOINT_TEMPERATURES_STRING, strlen(TEMPERATURE_SETPOINT_TEMPERATURES_STRING)) == 0) {
		t1 = 0.0;
		t2 = 0.0;
		t3 = 0.0;
		temps_read = 1;
		sscanf(line, TEMPERATURE_SETPOINT_TEMPERATURES_SCANF, &t1, &t2, &t3);
		// Ignore scanf failures. If it fails to read positions, they will be 0's.
		if (hb_read) {
			// Assign the position with b == 1 if any
			info->bed_temp = b1?t1:b2?t2:b3?t3:0;
			// Assign the first with b == 0 if any
			info->extruder_temp = !b1?t1:!b2?t2:!b3?t3:0;
		}
	} else if (strncmp(line, TEMPERATURE_HEATED_BED_STRING, strlen(TEMPERATURE_HEATED_BED_STRING)) == 0) {
		b1 = 0;
		b2 = 0;
		b3 = 0;
		hb_read = 1;
		sscanf(line, TEMPERATURE_HEATED_BED_SCANF, &b1, &b2, &b3);
		// Ignore scanf failures. If it fails to read positions, they will be 0's.
		if (temps_read) {
			// Assign the position with b == 1 if any
			info->bed_temp = b1?t1:b2?t2:b3?t3:0;
			// Assign the first with b == 0 if any
			info->extruder_temp = !b1?t1:!b2?t2:!b3?t3:0;
		}
	}
}

bool Simplify3DDialect::is_this_slicer_gcode(const char* line) {
	return strncmp(SIMPLIFY3D_INIT_HEADER, line, strlen(SIMPLIFY3D_INIT_HEADER)) == 0;
}

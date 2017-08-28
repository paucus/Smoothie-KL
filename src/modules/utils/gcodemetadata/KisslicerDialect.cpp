/*
 * KisslicerDialect.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: eai
 */

#include "KisslicerDialect.h"
#include <string.h>
#include <stdio.h>
#include <cmath>

#define KISSLICER_INIT_HEADER "; KISSlicer"
#define LAYER_HEIGHT_STRING "; layer_thickness_mm = "
#define PLASTIC_VOLUME_STRING "; Estimated Build Volume: "
#define BUILD_TIME_STRING "; Estimated Build Time: "
#define TEMPERATURE_STRING "; temperature_C = "
#define TEMPERATURE_HEATED_BED_STRING "; bed_C = "

#define LAYER_HEIGHT_SCANF "; layer_thickness_mm = %f"
#define PLASTIC_VOLUME_SCANF "; Estimated Build Volume: %f"
#define BUILD_TIME_SCANF "; Estimated Build Time: %f"
#define TEMPERATURE_SCANF "; temperature_C = %f"
#define TEMPERATURE_HEATED_BED_SCANF "; bed_C = %f"

KisslicerDialect::KisslicerDialect() {
}

KisslicerDialect::~KisslicerDialect() {
}


void KisslicerDialect::parse_line(SlicingInformation* info, const char* line) {
	if (strncmp(line, LAYER_HEIGHT_STRING, strlen(LAYER_HEIGHT_STRING)) == 0) {
		float lh = 0.0;
		sscanf(line, LAYER_HEIGHT_SCANF, &lh);
		// Express the value in microns
		info->layer_height = (int)roundf(lh*1000.0);
	} else if (strncmp(line, PLASTIC_VOLUME_STRING, strlen(PLASTIC_VOLUME_STRING)) == 0) {
		float plastic_volume = 0.0;
		sscanf(line, PLASTIC_VOLUME_SCANF, &plastic_volume);
		// The value is already in cm^3
		info->filament_used_vol_cm3 = plastic_volume;
	} else if (strncmp(line, BUILD_TIME_STRING, strlen(BUILD_TIME_STRING)) == 0) {
		float mins = 0;
		sscanf(line, BUILD_TIME_SCANF, &mins);
		info->print_time = mins*60;
	} else if (strncmp(line, TEMPERATURE_STRING, strlen(TEMPERATURE_STRING)) == 0) {
		float temp = 0;
		sscanf(line, TEMPERATURE_SCANF, &temp);
		info->extruder_temp = temp;
	} else if (strncmp(line, TEMPERATURE_HEATED_BED_STRING, strlen(TEMPERATURE_HEATED_BED_STRING)) == 0) {
		float temp = 0;
		sscanf(line, TEMPERATURE_HEATED_BED_SCANF, &temp);
		info->bed_temp = temp;
	}
}

bool KisslicerDialect::is_this_slicer_gcode(const char* line) {
	return strncmp(KISSLICER_INIT_HEADER, line, strlen(KISSLICER_INIT_HEADER)) == 0;
}

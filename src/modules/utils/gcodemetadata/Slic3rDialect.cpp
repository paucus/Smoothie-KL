/*
 * Slic3rDialect.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#include "Slic3rDialect.h"
#include <string.h>
#include <stdio.h>
#include <cmath>

#define LAYER_HEIGHT_STRING "; layer_height = "
#define FILAMENT_LENGTH_AND_VOL_STRING "; filament used = "
#define TEMPERATURE_HEATED_BED_STRING "; bed_temperature = "
#define TEMPERATURE_STRING "; temperature = "

#define LAYER_HEIGHT_SCANF "; layer_height = %f"
#define FILAMENT_LENGTH_AND_VOL_SCANF "; filament used = %fmm (%fcm3)"
#define TEMPERATURE_HEATED_BED_SCANF "; bed_temperature = %f"
#define TEMPERATURE_SCANF "; temperature = %f"

Slic3rDialect::Slic3rDialect() {
}

Slic3rDialect::~Slic3rDialect() {
}

void Slic3rDialect::parse_line(SlicingInformation* info, const char* line) {
	if (strncmp(line, LAYER_HEIGHT_STRING, strlen(LAYER_HEIGHT_STRING)) == 0) {
		float lh = 0.0;
		sscanf(line, LAYER_HEIGHT_SCANF, &lh);
		// Express the value in microns
		info->layer_height = (int)roundf(lh*1000.0);
	} else if (strncmp(line, FILAMENT_LENGTH_AND_VOL_STRING, strlen(FILAMENT_LENGTH_AND_VOL_STRING)) == 0) {
		float flength = 0.0;
		float fvol = 0.0;
		sscanf(line, FILAMENT_LENGTH_AND_VOL_SCANF, &flength, &fvol);
		// Express the value in meters. The volume is already in cm^3.
		info->filament_used_meters = flength/1000.0;
		info->filament_used_vol_cm3 = fvol;
	} else if (strncmp(line, TEMPERATURE_HEATED_BED_STRING, strlen(TEMPERATURE_HEATED_BED_STRING)) == 0) {
		float temp = 0.0;
		sscanf(line, TEMPERATURE_HEATED_BED_SCANF, &temp);
		info->bed_temp = temp;
	} else if (strncmp(line, TEMPERATURE_STRING, strlen(TEMPERATURE_STRING)) == 0) {
		float temp = 0.0;
		sscanf(line, TEMPERATURE_SCANF, &temp);
		info->extruder_temp = temp;
	}
}

bool Slic3rDialect::is_this_slicer_gcode(const char* line) {
	return strncmp(SLIC3R_INIT_HEADER, line, strlen(SLIC3R_INIT_HEADER)) == 0;
}

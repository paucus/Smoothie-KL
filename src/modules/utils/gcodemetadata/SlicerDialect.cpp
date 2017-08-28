/*
 * SlicerDialect.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#include "SlicerDialect.h"
#include "Slic3rDialect.h"
#include "Simplify3DDialect.h"
#include "KisslicerDialect.h"
#include <algorithm>

SlicerDialect::SlicerDialect() {
}

SlicerDialect::~SlicerDialect() {
}

SlicerDialect* SlicerDialect::identify_dialect(const char* line) {
	if (Slic3rDialect::is_this_slicer_gcode(line)) {
		return new Slic3rDialect();
	} else if (Simplify3DDialect::is_this_slicer_gcode(line)) {
		return new Simplify3DDialect();
	} else if (KisslicerDialect::is_this_slicer_gcode(line)) {
		return new KisslicerDialect();
	} else {
		return nullptr;
	}
}
unsigned int SlicerDialect::get_min_bytes_to_identify_slicer() {
	return std::max(std::max(Slic3rDialect::BYTES_TO_IDENTIFY_SLICER, Simplify3DDialect::BYTES_TO_IDENTIFY_SLICER), KisslicerDialect::BYTES_TO_IDENTIFY_SLICER);
}


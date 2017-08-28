/*
 * SlicingInformation.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: idlt
 */

#include "SlicingInformation.h"
const char* slicer_name_to_string(enum slicer_name n) {
	switch (n) {
	case SLIC3R: return "Slic3r";
	case SIMPLIFY3D: return "Simplify3D";
	case KISSLICER: return "Kisslicer";
	default: return "Unknown";
	}
}


SlicingInformation::SlicingInformation() : slicer(UNKNOWN_SLICER) {

}

SlicingInformation::~SlicingInformation() {
}


/*
 * SlicingInformation.h
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#ifndef SLICINGINFORMATION_H_
#define SLICINGINFORMATION_H_

#include <inttypes.h>
#include "opt_attr.h"
#include <time.h>

// If more slicers are added, remember to add them to slicer_name_to_string
// and the code to identify it in SlicerDialect::identify_dialect.
enum slicer_name {UNKNOWN_SLICER, SLIC3R, SIMPLIFY3D, KISSLICER};
const char* slicer_name_to_string(enum slicer_name n);

class SlicingInformation {
public:
	SlicingInformation();
	virtual ~SlicingInformation();

	enum slicer_name slicer;
	// In order to keep as much precision as possible, this value is expressed in microns.
	// This allows to hold values from 1 micron (0.001mm) to 65535 microns (65.5mms).
	struct opt_attr<uint16_t> layer_height;
	struct opt_attr<uint16_t> extruder_temp;
	struct opt_attr<uint16_t> bed_temp;
	struct opt_attr<float> filament_used_meters;
	struct opt_attr<float> filament_used_grams;
	struct opt_attr<float> filament_used_vol_cm3;
	struct opt_attr<time_t> print_time;
};

#endif /* SLICINGINFORMATION_H_ */

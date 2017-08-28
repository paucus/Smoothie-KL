/*
 * SlicerDialect.h
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#ifndef SLICERDIALECT_H_
#define SLICERDIALECT_H_

#include "SlicingInformation.h"

// WARNING: If you create a new slicer dialect you must also add the corresponding entries to
// get_min_bytes_to_identify_slicer and identify_dialect, and add it to the slicer_name enum.
class SlicerDialect {
public:
	SlicerDialect();
	virtual ~SlicerDialect();

	// Parses the line and populates the SlicingInformation object if any information is found.
	virtual void parse_line(SlicingInformation* info, const char* line) = 0;

	virtual unsigned int get_header_len() = 0;
	virtual unsigned int get_footer_len() = 0;
	virtual enum slicer_name get_slicer_name() = 0;

	static unsigned int get_min_bytes_to_identify_slicer();
	static SlicerDialect* identify_dialect(const char* line);
};

#endif /* SLICERDIALECT_H_ */

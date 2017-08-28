/*
 * GcodeMetadata.h
 *
 *  Created on: Feb 1, 2016
 *      Author: eai
 */

#ifndef GCODEMETADATA_H_
#define GCODEMETADATA_H_

#include "Module.h"

// This module allows the user to scan for some common gcode comments with
// extra metadata, like layer height, etc.
class GcodeMetadata: public Module {
public:
	GcodeMetadata();
	virtual ~GcodeMetadata();

	void on_module_loaded();
	void on_console_line_received(void * args);
};

#endif /* GCODEMETADATA_H_ */

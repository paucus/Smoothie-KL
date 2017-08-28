/*
 * FullGcodeAnalyzer.h
 *
 *  Created on: Jul 17, 2015
 *      Author: eai
 */

#ifndef FULLGCODEANALYZER_H_
#define FULLGCODEANALYZER_H_

#include "PrinterStateAnalyzer.h"

// This analyzer scans the whole file
class FullGcodeAnalyzer: public PrinterStateAnalyzer {
public:
	FullGcodeAnalyzer();
	FullGcodeAnalyzer(const float* homing_position);
	virtual ~FullGcodeAnalyzer();

	// Return true if the line must not be processed as g-code
	virtual bool prefilter(const char* line) { return false; };
	virtual void on_gcode(Gcode* gcode) = 0;
	virtual void begin() {};
	virtual void end() {};
	virtual int analyze(const char* file, void* result);
protected:
	unsigned long accum_bytes;
};

#endif /* FULLGCODEANALYZER_H_ */

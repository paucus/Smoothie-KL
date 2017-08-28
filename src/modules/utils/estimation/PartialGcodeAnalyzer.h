/*
 * PartialGcodeAnalyzer.h
 *
 *  Created on: Jul 17, 2015
 *      Author: eai
 */

#ifndef PARTIALGCODEANALYZER_H_
#define PARTIALGCODEANALYZER_H_

#include "PrinterStateAnalyzer.h"
#include <stdio.h>
#include <iterator>
#include "chunk.h"

class PartialGcodeAnalyzer: public PrinterStateAnalyzer {
public:
	PartialGcodeAnalyzer();
	PartialGcodeAnalyzer(const float* homing_position);
	virtual ~PartialGcodeAnalyzer();

	// Return true if the line must not be processed as g-code
	virtual bool prefilter(const char* line) { return false; };
	virtual void on_gcode(Gcode* gcode) = 0;
	virtual void begin() {};
	virtual void end() {};
	virtual void begin_chunk(const chunk_t& chunk) {};
	virtual void end_chunk(const chunk_t& chunk) {};
	virtual chunk_iterator& build_chunk_iterator() = 0;
	virtual int analyze(const char* file, void* result);
	virtual int analyze_chunk(FILE* f, const chunk_t& chunk);

protected:
	// As we are not traversing the whole file, accum_bytes represent only the current position in
	// it.
	unsigned long accum_bytes;
};

#endif /* PARTIALGCODEANALYZER_H_ */

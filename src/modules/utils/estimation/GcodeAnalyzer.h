/*
 * GcodeAnalyzer.h
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#ifndef GCODEANALYZER_H_
#define GCODEANALYZER_H_

#include "Gcode.h"

#define ANALYZE_OK  0
#define ANALYZE_ERR -1

// using the same buffer size of the player
#define GCODE_ANALYZER_BUFFSIZE 130

class GcodeAnalyzer {
public:
	GcodeAnalyzer();
	virtual ~GcodeAnalyzer();

	virtual int analyze(const char* file, void* result);

	unsigned long total_file_size;
	void* current_result;
};


#endif /* GCODEANALYZER_H_ */

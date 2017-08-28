/*
 * TimeArrayGcodeAnalyzer.h
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#ifndef FULLTIMEARRAYGCODEANALYZER_H_
#define FULLTIMEARRAYGCODEANALYZER_H_

#include "FullGcodeAnalyzer.h"
#include "TimeArray.h"

class FullTimeArrayGcodeAnalyzer: public FullGcodeAnalyzer {
public:
	FullTimeArrayGcodeAnalyzer();
	FullTimeArrayGcodeAnalyzer(const float* homing_position);
	virtual ~FullTimeArrayGcodeAnalyzer();

	bool prefilter(const char* line);
	void on_gcode(Gcode* gcode);
	void begin();
	void end();
protected:

	// intermediate attributes to generate the array
	float accum_seconds;

	INDEX_TYPE current_index_in_time_array;	// our position in the time_samples_array
};

#endif /* FULLTIMEARRAYGCODEANALYZER_H_ */

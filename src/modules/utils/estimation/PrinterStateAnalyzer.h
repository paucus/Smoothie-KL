/*
 * PrinterStateAnalyzer.h
 *
 *  Created on: Jul 17, 2015
 *      Author: eai
 */

#ifndef PRINTERSTATEANALYZER_H_
#define PRINTERSTATEANALYZER_H_

#include "GcodeAnalyzer.h"

#include "nuts_bolts.h"
#include <math.h>
#include <string.h>

#define E_AXIS 3
#define F_AXIS 4

class PrinterStateAnalyzer: public GcodeAnalyzer {
public:
	PrinterStateAnalyzer();
	PrinterStateAnalyzer(const float* homing_position);
	virtual ~PrinterStateAnalyzer();

	// Applies the gcode and returns the time taken to run it
	float process_gcode(Gcode* gcode);

protected:
	// theorical printer state if the gcode was executed
	float last_axis_value[5] = {0,0,0,0,0};
	float homing_position[3] = {0,0,0};

	struct {
		bool absolute:1;
		bool ext_absolute:1;
	};
};


inline float vec_distance(float* v1, float* v2){
	float distance = 0;
	for (int c = X_AXIS; c <= Z_AXIS; c++)
		distance += pow(v1[c] - v2[c], 2);
	distance = sqrt(distance);
	return distance;
}
inline void copy_vec_x_to_e(float* to, const float* from){
	for (int c = X_AXIS; c <= E_AXIS; c++) {
		to[c] = from[c];
	}
}
inline void copy_vec_x_to_z(float* to, const float* from){
	for (int c = X_AXIS; c <= Z_AXIS; c++) {
		to[c] = from[c];
	}
}
inline bool param_ends_in_position(const char* line, unsigned int index){
	char c = line[index];
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
inline bool is_one_of_these_gcodes(const char* line, const char** gcodes_to_analyze) {
	const char** p = gcodes_to_analyze;
	while (*p){
		size_t len_p = strlen(*p);
		if (strncmp(line, *p, len_p) == 0 && param_ends_in_position(line, len_p)) {
			return true;
		}
		p++;
	}
	return false;
}

#endif /* PRINTERSTATEANALYZER_H_ */

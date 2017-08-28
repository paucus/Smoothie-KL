/*
 * FullTimeArrayGcodeAnalyzer.cpp
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#include "FullTimeArrayGcodeAnalyzer.h"
#include "utils.h"
#include <string.h>

// WARNING: If you add an M or T code, remember to adjust the if statement in prefilter, or better,
// add another if for M/T codes and add a new line, so that list scanning is faster (it wouldn't
// need to scan Gcodes that we already know that will never match).
static const char* gcodes_to_analyze[] = {"G0", "G1", "G4", "G28", "G82", "G83", "G90", "G91", "G92", nullptr};

#define min(a,b) ((a<b)?a:b)

FullTimeArrayGcodeAnalyzer::FullTimeArrayGcodeAnalyzer() : FullGcodeAnalyzer() {
	accum_seconds = 0;
	accum_bytes = 0;
	current_index_in_time_array = 0;
}

FullTimeArrayGcodeAnalyzer::FullTimeArrayGcodeAnalyzer(const float* homing_position) : FullGcodeAnalyzer(homing_position) {
	accum_seconds = 0;
	accum_bytes = 0;
	current_index_in_time_array = 0;
}

FullTimeArrayGcodeAnalyzer::~FullTimeArrayGcodeAnalyzer() {
}

void FullTimeArrayGcodeAnalyzer::begin(){
	accum_seconds = 0;
	current_index_in_time_array = 0;
	this->absolute = true;
	this->ext_absolute = true;
	((TimeArray*)current_result)->time_samples[0] = 0.0;	// the first position is always 0
}
bool FullTimeArrayGcodeAnalyzer::prefilter(const char* line){
	// check if we must store a new item in the array
	int index = min( (int)((accum_bytes * NUMBER_OF_SAMPLES) / total_file_size), NUMBER_OF_SAMPLES - 1);
	while (index != current_index_in_time_array){	// note: used while in case a line is too long in comparison with the rest of the gcode
		// store the number of seconds taken in this lapse
		((TimeArray*)current_result)->time_samples[current_index_in_time_array+1] = accum_seconds;
		current_index_in_time_array++;
	}

	// now, check if we must filter the current gcode
	line = trim_left_cstr(line);
	if (line[0] == 'N'){
		// skip N
		line = trim_left_cstr(move_to_first_space_cstr(line));
	}
	if (line[0] == 'G') {
		// we are only interested in a few gcodes
		return !is_one_of_these_gcodes(line, gcodes_to_analyze);
	}
	return true;	// filter the rest of g-codes
}

void FullTimeArrayGcodeAnalyzer::end(){
	// check if we must store a new item in the array
	while (NUMBER_OF_SAMPLES != current_index_in_time_array){	// note: used while in case a line is too long in comparison with the rest of the gcode
		// store the number of seconds taken in this lapse
		((TimeArray*)current_result)->time_samples[current_index_in_time_array+1] = accum_seconds;
		current_index_in_time_array++;
	}
}

void FullTimeArrayGcodeAnalyzer::on_gcode(Gcode* gcode){
	accum_seconds += this->process_gcode(gcode);
}


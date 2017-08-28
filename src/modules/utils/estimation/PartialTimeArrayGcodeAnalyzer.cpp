/*
 * PartialTimeArrayGcodeAnalyzer.cpp
 *
 *  Created on: Jul 20, 2015
 *      Author: eai
 */

#include "PartialTimeArrayGcodeAnalyzer.h"

#include "utils.h"
#include <string.h>
#include "GcodeUtils.h"
#include "TimeArray.h"

// WARNING: If you add an M or T code, remember to adjust the if statement in prefilter, or better,
// add another if for M/T codes and add a new line, so that list scanning is faster (it wouldn't
// need to scan Gcodes that we already know that will never match).
static const char* gcodes_to_analyze[] = {"G0", "G1", "G4", "G28", "G82", "G83", "G90", "G91", "G92", nullptr};

#define min(a,b) ((a<b)?a:b)

PartialTimeArrayGcodeAnalyzer::PartialTimeArrayGcodeAnalyzer() : PartialGcodeAnalyzer(), known_coordinates(1 << Z_AXIS), it(0,0,0) {
	accum_seconds = 0;
	current_index_in_time_array = 0;
	chunk_accum_bytes_start = 0;
	chunk_accum_bytes_start_known = false;
	chunk_accum_bytes_end = 0;
	chunk_accum_seconds = 0;
	segment_accum_seconds = 0;
	segment_accum_bytes = 0;
}

PartialTimeArrayGcodeAnalyzer::PartialTimeArrayGcodeAnalyzer(const float* homing_position) : PartialGcodeAnalyzer(homing_position), known_coordinates(1 << Z_AXIS), it(0,0,0) {
	accum_seconds = 0;
	current_index_in_time_array = 0;
	chunk_accum_bytes_start = 0;
	chunk_accum_bytes_start_known = false;
	chunk_accum_bytes_end = 0;
	chunk_accum_seconds = 0;
	segment_accum_seconds = 0;
	segment_accum_bytes = 0;
}

PartialTimeArrayGcodeAnalyzer::~PartialTimeArrayGcodeAnalyzer() {
}

void PartialTimeArrayGcodeAnalyzer::begin(){
	accum_seconds = 0;
	current_index_in_time_array = 0;
	this->absolute = true;
	this->ext_absolute = true;
	((TimeArray*)current_result)->time_samples[0] = 0.0;	// the first position is always 0

	chunk_accum_bytes_start = 0;
	chunk_accum_bytes_start_known = false;
	chunk_accum_bytes_end = 0;
	chunk_accum_seconds = 0;
	segment_accum_seconds = 0;
	segment_accum_bytes = 0;
}

void PartialTimeArrayGcodeAnalyzer::begin_chunk(const chunk_t& chunk) {
	chunk_accum_bytes_start = 0;
	chunk_accum_bytes_start_known = false;
	chunk_accum_bytes_end = 0;
	chunk_accum_seconds = 0;
	known_coordinates = 1 << Z_AXIS;	// consider Z known
	chunk_accum_bytes_start_known = false;
}
void PartialTimeArrayGcodeAnalyzer::end_chunk(const chunk_t& chunk) {
	// estimate the total number of seconds it would take to complete the segment by doing an extrapolation
	segment_accum_bytes += chunk_accum_bytes_start_known?(chunk_accum_bytes_end - chunk_accum_bytes_start):0;
	segment_accum_seconds += chunk_accum_seconds;
}

chunk_iterator& PartialTimeArrayGcodeAnalyzer::build_chunk_iterator() {
	it = UniformSegmentsIterator(this->total_file_size, PARTIAL_ANALYZER_NUMBER_OF_SECTORS_TO_ANALYZE*512/SECTORS_GROUP_SIZE, SECTORS_GROUP_SIZE);
	return it;
}

bool PartialTimeArrayGcodeAnalyzer::prefilter(const char* line){
	// check if we must store a new item in the array
	int index = min( (int)((accum_bytes * NUMBER_OF_SAMPLES) / total_file_size), NUMBER_OF_SAMPLES - 1);
	while (index != current_index_in_time_array){	// note: used while in case a line is too long in comparison with the rest of the gcode
		// estimate extrapolating the number of seconds based on the information retrieved from the chunks
		if (segment_accum_bytes > 0) {
			float segment_size = total_file_size / NUMBER_OF_SAMPLES;
			accum_seconds += (segment_accum_seconds * segment_size) / segment_accum_bytes;
		}
		// store the number of seconds taken in this lapse
		(*(TimeArray*)current_result)[current_index_in_time_array+1] = accum_seconds;
		current_index_in_time_array++;
		segment_accum_seconds = 0;
		segment_accum_bytes = 0;
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

void PartialTimeArrayGcodeAnalyzer::end(){
	// check if we must store a new item in the array
	while (NUMBER_OF_SAMPLES != current_index_in_time_array){	// note: used while in case a line is too long in comparison with the rest of the gcode
		// estimate extrapolating the number of seconds based on the information retrieved from the chunks
		if (segment_accum_bytes > 0) {
			float segment_size = total_file_size / NUMBER_OF_SAMPLES;
			accum_seconds += (segment_accum_seconds * segment_size) / segment_accum_bytes;
		}
		// store the number of seconds taken in this lapse
		((TimeArray*)current_result)->time_samples[current_index_in_time_array+1] = accum_seconds;
		current_index_in_time_array++;

		segment_accum_seconds = 0;
		segment_accum_bytes = 0;
	}
}

bool PartialTimeArrayGcodeAnalyzer::all_coordinates_are_known() {
	return known_coordinates.all();
}

void PartialTimeArrayGcodeAnalyzer::search_for_remaining_coordinates_to_be_known(Gcode* gcode) {
	if (gcode->g == 28) {
		// This logic is similar to the one in PrinterStateAnalyzer.cpp:on_gcode for G28.

		// Ignore the time a home takes as it's unpredictable and it won't be counted for the estimation
		// We only need to update our position in space
		bool homed_any_axis_in_particular = gcode->has_letter('X') && gcode->has_letter('Y') && gcode->has_letter('Z');

		if (homed_any_axis_in_particular || gcode->has_letter('Z')) {
			// Home all or Z ends in 0,0,0,
			for (int c = X_AXIS; c <= Z_AXIS; c++) {
				last_axis_value[c] = 0;
				known_coordinates[c] = 1;
			}
		} else {
			for (int c = X_AXIS; c <= Z_AXIS; c++) {
				bool homed_axis = gcode->has_letter(c + 'X') || !homed_any_axis_in_particular;
				if (homed_axis) {
					last_axis_value[c] = homing_position[c];
					known_coordinates[c] = 1;
				}
			}
		}

	} else if (gcode->g == 0 || gcode->g == 1 || gcode->g == 92) {
		if (gcode->has_letter('X')) {
			known_coordinates[X_AXIS] = 1;
			last_axis_value[X_AXIS] = gcode->get_value('X');
		}
		if (gcode->has_letter('Y')){
			known_coordinates[Y_AXIS] = 1;
			last_axis_value[Y_AXIS] = gcode->get_value('Y');
		}
		if (gcode->has_letter('E')){
			known_coordinates[E_AXIS] = 1;
			last_axis_value[E_AXIS] = gcode->get_value('E');
		}
		if (gcode->has_letter('F')){
			known_coordinates[F_AXIS] = 1;
			last_axis_value[F_AXIS] = gcode->get_value('F');
		}
	}
}

void PartialTimeArrayGcodeAnalyzer::on_gcode(Gcode* gcode){
	if (!all_coordinates_are_known()) {
		search_for_remaining_coordinates_to_be_known(gcode);
		// accum_bytes points at the end of the current line.
		// Once all axis are known, this value will be the beginning of the gcode line where all
		// axis are known.
		chunk_accum_bytes_start = accum_bytes;
	} else {
		if (!chunk_accum_bytes_start_known) {
			// Now that we know where the chunk start, we mustn't update that position any more.
			chunk_accum_bytes_start_known = true;
		}
		// update every turn the end of the chunk, so that at the end, only the executed gcodes get counted.
		chunk_accum_bytes_end = accum_bytes;

		// Discard the z displacement.
		// A trick to do this is just setting the current Z value to the one of the g-code. This is
		// a workaround. Probably there are better ways to do this.
		if (gcode->has_letter('Z') && (gcode->g == 0 || gcode->g == 1 || gcode->g == 92))
			last_axis_value[Z_AXIS] = gcode->get_value('Z');

		chunk_accum_seconds += this->process_gcode(gcode);
	}
}


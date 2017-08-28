/*
 * PrecalculationEstimation.cpp
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#include "PrecalculationEstimation.h"

#include "FullTimeArrayGcodeAnalyzer.h"
#include "PartialTimeArrayGcodeAnalyzer.h"
#include "utils.h"

PrecalculationEstimation::PrecalculationEstimation() {
	this->total_file_size = 0;	// unknown

	current_estimation_timestamp = 0;
	analysis_was_performed = false;
	current_remaining_time_est = INFINITE_TIME;
}

PrecalculationEstimation::~PrecalculationEstimation() {
}

unsigned long PrecalculationEstimation::calculate_remaining_time(float extrusion_multiplier){
	if (!analysis_was_performed)
		return 0;

	detailed_progress p = get_sd_card_playing_progress();

	unsigned long t_acum = p.effective_secs;
	unsigned long b_acum = p.played_bytes;

	if (current_estimation_timestamp == 0 || t_acum - current_estimation_timestamp >= ESTIMATION_INTERVAL){
		// Get the number of byte vs time segments. A segment is the interval of time between two measures.
		unsigned int array_count = time_array.get_time_array_number_of_elements();
		// there are always at least two elements:
		// time_array[0] = 0
		// time_array[array_count - 1] = total_time
		const float* ta = time_array.get_time_array();
		// We want the index to run from 0 to (array_count-2) really, so that i+1 at most is (array_count-1).
		unsigned int i = (b_acum * (array_count-1)) / total_file_size;
		if (i >= array_count-1) i = array_count - 2;

		// measured speed
		//float speed = ((float)(t_acum - begin_meas_t2)) / (b_acum - begin_meas_b2); // measured speed since the checkpoint before the last (seconds/bytes)

		// theorical speed
		float speed = (ta[i+1] - ta[i])*(array_count-1) / ( total_file_size ); // expected speed

		// first calculate the number of bytes that will have been processed on next segment.
		unsigned long bytes_on_next_segment = (total_file_size * (i+1)) / (array_count - 1);
		// The theorical remaining time plus what remains from this current segment
		current_remaining_time_est = (unsigned long)((ta[array_count-1] - ta[i+1]) + (bytes_on_next_segment - b_acum) * speed);

		current_estimation_timestamp = t_acum;
	}

	// adjust by feedrate %
	unsigned int dt = t_acum - current_estimation_timestamp;
	if (extrusion_multiplier * dt > current_remaining_time_est) {
		return 0;	// should be finishing in a couple of seconds at most.
	} else if (extrusion_multiplier <= 0) {
		// mh, something is wrong. Return infinite just in case.
		return INFINITE_TIME;
	} else {
		return current_remaining_time_est / extrusion_multiplier - dt;
	}
}

void PrecalculationEstimation::preprocess_file(const char* gcode_file){
	current_estimation_timestamp = 0;

	this->total_file_size = flen(gcode_file);

	GcodeAnalyzer* analyzer;
	// FIXME mhh, this logic shouldn't go here. It shouldn't be this class' responsibility.
	if (this->total_file_size <= PARTIAL_ANALYZER_NUMBER_OF_SECTORS_TO_ANALYZE*512) {
		analyzer = new FullTimeArrayGcodeAnalyzer();
	} else {
		analyzer = new PartialTimeArrayGcodeAnalyzer();
	}

	if (analyzer->analyze(gcode_file, &time_array) == ANALYZE_OK) {
		analysis_was_performed = true;
	} else {
		analysis_was_performed = false;
	}

	delete analyzer;
}

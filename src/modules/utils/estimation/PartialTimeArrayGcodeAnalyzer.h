/*
 * PartialTimeArrayGcodeAnalyzer.h
 *
 *  Created on: Jul 20, 2015
 *      Author: eai
 */

#ifndef PARTIALTIMEARRAYGCODEANALYZER_H_
#define PARTIALTIMEARRAYGCODEANALYZER_H_

#include "PartialGcodeAnalyzer.h"
#include <bitset>
#include "UniformSegmentsIterator.h"
#include "TimeArray.h"

// 1280 sectors take more or less 5 seconds to analyze. This number has the advantage that is both
// divisible by 10 (the number of segments in each file; see TimeArray NUMBER_OF_SAMPLES) and 32,
// (the number of sectors per chunk; see SECTORS_GROUP_SIZE).
#define PARTIAL_ANALYZER_NUMBER_OF_SECTORS_TO_ANALYZE 1280
// In order to reduce the number of lost lines due to chunk beginning and ending, it's better to
// make chunks of multiple disk sectors. 16KB chunks seemed to be a good number according to tests.
#define SECTORS_GROUP_SIZE (32*512)

class PartialTimeArrayGcodeAnalyzer: public PartialGcodeAnalyzer {
public:
	PartialTimeArrayGcodeAnalyzer();
	PartialTimeArrayGcodeAnalyzer(const float* homing_position);
	virtual ~PartialTimeArrayGcodeAnalyzer();

	bool prefilter(const char* line);
	void on_gcode(Gcode* gcode);
	void begin();
	void end();
	void begin_chunk(const chunk_t& chunk);
	void end_chunk(const chunk_t& chunk);
	chunk_iterator& build_chunk_iterator();

protected:

	// intermediate attributes to generate the array
	float accum_seconds;

	float chunk_accum_seconds;
	bool chunk_accum_bytes_start_known;
	unsigned long chunk_accum_bytes_start;
	unsigned long chunk_accum_bytes_end;
	float segment_accum_seconds;
	float segment_accum_bytes;

	INDEX_TYPE current_index_in_time_array;	// our position in the time_samples_array

	// This variable will be initialized with 1 << Z_AXIS, because we are not interested in Z, and
	// removing it from the bitset would complicate the code much more than just leaving it.
	std::bitset<5> known_coordinates;
	UniformSegmentsIterator it;

	bool all_coordinates_are_known();
	void search_for_remaining_coordinates_to_be_known(Gcode* gcode);
};

#endif /* PARTIALTIMEARRAYGCODEANALYZER_H_ */

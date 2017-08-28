/*
 * PartialGcodeAnalyzer.cpp
 *
 *  Created on: Jul 17, 2015
 *      Author: eai
 */

#include "PartialGcodeAnalyzer.h"
#include "StreamOutput.h"
#include "TimeArray.h"

#define min(a,b) ((a<b)?a:b)

PartialGcodeAnalyzer::PartialGcodeAnalyzer() {
	accum_bytes = 0;
}
PartialGcodeAnalyzer::PartialGcodeAnalyzer(const float* homing_position) : PrinterStateAnalyzer(homing_position) {
	accum_bytes = 0;
}

PartialGcodeAnalyzer::~PartialGcodeAnalyzer() {
}

int PartialGcodeAnalyzer::analyze_chunk(FILE* f, const chunk_t& chunk) {
	char buf[GCODE_ANALYZER_BUFFSIZE];
	unsigned int chunk_read = 0;

	unsigned long chunk_start = chunk.start;
	unsigned int length = chunk.length;

	accum_bytes = chunk_start;
	fseek(f, chunk_start, SEEK_SET);

	// Skip the first line, as it is very likely to be incomplete, except if it is the first one.
	if (chunk_start > 0) {
		fgets(buf, min(sizeof(buf), length), f);
		int len = strnlen(buf, GCODE_ANALYZER_BUFFSIZE);
		accum_bytes += len;
		chunk_read += len;
	}

	while(fgets(buf, min(sizeof(buf), length - chunk_read), f) != NULL) {
		unsigned int len = strnlen(buf, GCODE_ANALYZER_BUFFSIZE);
		accum_bytes += len;
		chunk_read += len;
		if(len == 0 && length - chunk_read == 1) break;	// In some cases this leads to no more readable lines
		if(len == min(sizeof(buf), length - chunk_read)) continue;	// no null char
		if(len == 0) continue;
		if(buf[len - 1] == '\n') {	// If it is the last line, discard it too, as it is very likely to be incomplete
			if (!prefilter(buf)) {
				Gcode gcode(buf, &(StreamOutput::NullStream), false);
				on_gcode(&gcode);
			}
		}
	}
	return ANALYZE_OK;
}

int PartialGcodeAnalyzer::analyze(const char* file, void* result){
	int res;
	if ((res = PrinterStateAnalyzer::analyze(file, result)) != ANALYZE_OK)
		return res;

	FILE* f = fopen(file, "rt");
	if (!f){
		return ANALYZE_ERR;
	}

	accum_bytes = 0;
	begin();

	for (chunk_iterator& it = build_chunk_iterator(); it.has_more(); ++it) {
		chunk_t chunk = *it;	// first value = chunk beginning in bytes, second = chunk size
		begin_chunk(chunk);
		int result = analyze_chunk(f, chunk);
		if (result != ANALYZE_OK) {
			fclose(f);
			return ANALYZE_ERR;
		}
		end_chunk(chunk);
	}

	end();

	fclose(f);

	return ANALYZE_OK;
}

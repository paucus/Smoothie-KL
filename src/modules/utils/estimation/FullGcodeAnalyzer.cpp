/*
 * FullGcodeAnalyzer.cpp
 *
 *  Created on: Jul 17, 2015
 *      Author: eai
 */

#include "FullGcodeAnalyzer.h"
#include "StreamOutput.h"
#include <stdio.h>
#include <strings.h>

FullGcodeAnalyzer::FullGcodeAnalyzer() {
	accum_bytes = 0;
}
FullGcodeAnalyzer::FullGcodeAnalyzer(const float* homing_position) : PrinterStateAnalyzer(homing_position) {
	accum_bytes = 0;
}

FullGcodeAnalyzer::~FullGcodeAnalyzer() {
}

int FullGcodeAnalyzer::analyze(const char* file, void* result){
	int res;
	if ((res = PrinterStateAnalyzer::analyze(file, result)) != ANALYZE_OK)
		return res;

	char buf[GCODE_ANALYZER_BUFFSIZE]; // lines upto 128 characters are allowed, anything longer is discarded

	FILE* f = fopen(file, "rt");
	if (!f){
		return ANALYZE_ERR;
	}

	accum_bytes = 0;
	begin();

	while(fgets(buf, sizeof(buf), f) != NULL) {
		int len = strnlen(buf, GCODE_ANALYZER_BUFFSIZE);
		// keep count of the total bytes already sent
		accum_bytes += len;
		if (len == GCODE_ANALYZER_BUFFSIZE) continue;	// no null char
		if(len == 0) continue;
		if(buf[len - 1] == '\n') {
			if (!prefilter(buf)) {
				Gcode gcode(buf, &(StreamOutput::NullStream), false);
				on_gcode(&gcode);
			}
		}
	}

	end();

	fclose(f);

	return ANALYZE_OK;
}

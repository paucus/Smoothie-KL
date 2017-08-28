/*
 * GcodeAnalyzer.cpp
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#include "GcodeAnalyzer.h"
#include "StreamOutput.h"
#include <stdio.h>
#include "utils.h"

GcodeAnalyzer::GcodeAnalyzer() {
	total_file_size = 0;
	current_result = nullptr;
}

GcodeAnalyzer::~GcodeAnalyzer() {
}

int GcodeAnalyzer::analyze(const char* file, void* result){
	current_result = result; // Store the pointer to ease the code later.

	long tmp = flen(file);
	if (tmp < 0)
		return ANALYZE_ERR;

	total_file_size = tmp;

	return ANALYZE_OK;
}

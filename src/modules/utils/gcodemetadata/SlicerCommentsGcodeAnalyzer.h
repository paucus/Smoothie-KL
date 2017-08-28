/*
 * SlicerCommentsGcodeAnalyzer.h
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#ifndef SLICERCOMMENTSGCODEANALYZER_H_
#define SLICERCOMMENTSGCODEANALYZER_H_

#include "PartialGcodeAnalyzer.h"
#include "SlicerDialect.h"
#include "SlicingInformation.h"

class comments_analyzer_chunk_iterator;

class SlicerCommentsGcodeAnalyzer : public PartialGcodeAnalyzer {
public:
	SlicerCommentsGcodeAnalyzer(); //SlicingInformation* info
	virtual ~SlicerCommentsGcodeAnalyzer();

	// Return true if the line must not be processed as g-code
	virtual void begin();
	virtual bool prefilter(const char* line);
	virtual void end();
	virtual void on_gcode(Gcode* gcode) {};
	virtual chunk_iterator& build_chunk_iterator();
	virtual int analyze(const char* file, void* result);
private:
	SlicerDialect* dialect;
	SlicingInformation* info;
	comments_analyzer_chunk_iterator* it;
};

#endif /* SLICERCOMMENTSGCODEANALYZER_H_ */

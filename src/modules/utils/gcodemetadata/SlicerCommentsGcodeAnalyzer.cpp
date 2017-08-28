/*
 * SlicerCommentsGcodeAnalyzer.cpp
 *
 *  Created on: Jan 15, 2016
 *      Author: eai
 */

#include "SlicerCommentsGcodeAnalyzer.h"

#include "chunk.h"

class comments_analyzer_chunk_iterator : public chunk_iterator {
public:
	comments_analyzer_chunk_iterator(unsigned int total_file_size, unsigned int first_chunk_size) : i(0), total_file_size(total_file_size), first_chunk_size(first_chunk_size), dialect(nullptr) {}

	void set_slicer_dialect(SlicerDialect* dialect) { this->dialect = dialect;};

	chunk_t operator *() {
		if (i == 0) {
			return {0, first_chunk_size};
		} else if (i == 1) {
			if (dialect) {
				return {0, std::min(dialect->get_header_len(), total_file_size)};
			} else {
				return {0, 0};
			}
		} else {
			if (dialect) {
				if (total_file_size > dialect->get_footer_len()) {
					return {total_file_size - dialect->get_footer_len(), dialect->get_footer_len()};
				} else {
					return {0, total_file_size};
				}
			} else {
				return {0, 0};
			}
		}
	};
	chunk_iterator& operator ++(){
		i++;
		return *this;
	};
	bool has_more() {
		return i < 3;
	};

	bool operator != (const comments_analyzer_chunk_iterator& it) {
	    return this->i != it.i;
	}
	bool operator == (const comments_analyzer_chunk_iterator& it) {
	    return this->i == it.i;
	}
	const comments_analyzer_chunk_iterator end() {
	    return comments_analyzer_chunk_iterator(3);
	}


private:
	// Private constructor for the end method
	comments_analyzer_chunk_iterator(unsigned int i) : i(i), total_file_size(0), first_chunk_size(0), dialect(nullptr) {}
	int i;
	unsigned int total_file_size;
	unsigned int first_chunk_size;
	SlicerDialect* dialect;
};



//SlicerCommentsGcodeAnalyzer::SlicerCommentsGcodeAnalyzer(SlicingInformation* info) : dialect(nullptr), info(info), it(nullptr) {
SlicerCommentsGcodeAnalyzer::SlicerCommentsGcodeAnalyzer() : dialect(nullptr), info(nullptr), it(nullptr) {
}

SlicerCommentsGcodeAnalyzer::~SlicerCommentsGcodeAnalyzer() {
}

void SlicerCommentsGcodeAnalyzer::begin() {
	if (this->dialect) {
		delete this->dialect;
		this->dialect = nullptr;
	}
	if (this->it) {
		delete this->it;
		this->it = nullptr;
	}
}
int SlicerCommentsGcodeAnalyzer::analyze(const char* file, void* result) {
	this->info = static_cast<SlicingInformation*>(result);
	return PartialGcodeAnalyzer::analyze(file, result);
}

bool SlicerCommentsGcodeAnalyzer::prefilter(const char* line){
	if (!dialect) {
		// No dialect identified yet.
		dialect = SlicerDialect::identify_dialect(line);
		// Set the dialect to the chunk iterator
		it->set_slicer_dialect(dialect);
		if (dialect) {
			info->slicer = dialect->get_slicer_name();
		}
	} else {
		dialect->parse_line(info, line);
	}
	// We are not interested in parsing G-Code. Return always false.
	return false;
}
void SlicerCommentsGcodeAnalyzer::end() {
	if (this->dialect) {
		delete this->dialect;
		this->dialect = nullptr;
	}
	if (this->it) {
		delete this->it;
		this->it = nullptr;
	}
}
chunk_iterator& SlicerCommentsGcodeAnalyzer::build_chunk_iterator(){
	// G code comments are usually at the beginning and at the end.
	// The procedure to determine chuncks is the following:
	// Create 3 chunks:
	// * The first just a few bytes long to identify the dialect.
	// * If no dialect is detected, the two remaining are empty.
	// * If a dialect is detected, it determines how much to read from the
	//   beginning and the end of the g-code.
	if (it) {
		delete it;
	}
	it = new comments_analyzer_chunk_iterator(this->total_file_size, SlicerDialect::get_min_bytes_to_identify_slicer());
	return *it;
}


/*
 * CommandParserStream.cpp
 *
 *  Created on: Feb 19, 2015
 *      Author: abialet
 */

#include "CommandParserStream.h"
//#include "LCD4DMessageScreen.h"

parser_status_info_t CommandParserStream::info;
static bool file_opened = false;
static bool file_selected = false;

CommandParserStream::CommandParserStream() {
}

CommandParserStream::~CommandParserStream() {
}

int CommandParserStream::_putc(int c) {
	return THEKERNEL->streams->printf("%c", c);
}

int CommandParserStream::_getc(void) {
	return 0;
}

static inline const char* trim_begin(const char* s) {
	for (;isspace(*s); ++s) {
	}
	return s;
}
static inline const char* trim_end(const char* s) {
	for (;isspace(*s); ++s) {
	}
	if (!*s)
		return s;

	const char* se = s;
	for (;*s; ++s) {
		if (!isspace(*s)){
			se = s;
		}
	}
	se++;
	return se;
}

int CommandParserStream::puts(const char* str) {
	/* We read the file list, and save results to a list of strings */
	if (strncmp("Begin file list", str, 15) == 0) {
		reading_files = 1;
		filenames.clear();
//	} else if (strncmp("cold extrusion", str, 14) == 0) {
//		lcd_screens.message_screen->warn_cold_extrusion();
	} else if (strncmp("File opened:", str, 12) == 0) {
		file_opened = true;
	} else if (strncmp("File selected", str, 13) == 0) {
		file_selected = true;
	} else if (strncmp("Could not open directory", str, 24) == 0) {
		/* can't list dir...*/
	} else if (strncmp("End file list", str, 13) == 0) {
		reading_files = 0;
		fileiterator = filenames.begin();
	} else if (reading_files && strncmp(".", str, 1) == 0) {
		// ignore .files
	} else if (reading_files) {
		filenames.push_back(string(trim_begin(str), trim_end(str)));
	} else if (strlen(str) > 18 && str[8] == '|' && str[17] == '|') {
		//We can be "pretty sure" that this is a response to M827...
		strncpy(info.elapsed_time, str, 8);
		strncpy(info.remaining_time, str + 9, 8);

		// now it gets trickier, since the status report, can have a variable number of digits for percentage..
		int nextindex = 0;
		char number[4];
		for (int i = 18; i < 21; i++) {
			if (str[i] == '|') {
				number[i - 18] = '\0';
				nextindex = i + 1;
				break;
			} else {
				number[i - 18] = str[i];
			}
		}

		info.total_time_known = (number[0] != ' ');
		if (info.total_time_known) {
			info.percentage_complete = atoi(number);
		}
		strcpy(info.state, str + nextindex);
	}

	return strlen(str);
}

void CommandParserStream::print_file_list() {
	vector<std::string>::iterator i;
	for (i = filenames.begin(); i != filenames.end(); ++i) {
		THEKERNEL->streams->printf("%s\n", i->c_str());
	}
}

int CommandParserStream::has_file_list() {
	return filenames.size();
}

bool CommandParserStream::file_was_selected() {
	bool ret = file_opened & file_selected;
	file_opened = false;
	file_selected = false;

	return ret;
}

const char* CommandParserStream::get_current_filename() {
	return fileiterator->c_str();
}

const char* CommandParserStream::get_next_filename() {
	fileiterator++;
	const char* ret = fileiterator->c_str();

	if (fileiterator == filenames.end())
		fileiterator = filenames.begin();

	return ret;
}

const char* CommandParserStream::get_previous_filename() {
	if (fileiterator == filenames.begin())
		fileiterator = filenames.end();

	fileiterator--;

	return fileiterator->c_str();
}

unsigned int CommandParserStream::get_number_of_files() {
	return filenames.size();
}

bool CommandParserStream::has_status_info() {
	return info.elapsed_time != NULL;
}

parser_status_info_t CommandParserStream::get_status_info() {
//	parser_status_info_t to = info;
//
//	/* Invalidate status info */
//	info.elapsed_time = NULL;

	return info;
}

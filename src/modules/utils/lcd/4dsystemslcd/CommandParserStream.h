/*
 * CommandParserStream.h
 *
 *  Created on: Feb 19, 2015
 *      Author: abialet
 */

#ifndef COMMANDPARSERSTREAM_H_
#define COMMANDPARSERSTREAM_H_

#include "libs/StreamOutput.h"
#include "Kernel.h"
#include "StreamOutputPool.h"
#include <list>
#include "PrintStatus.h"
#include <stdlib.h>

typedef struct {
		bool total_time_known;
		char elapsed_time[9];
		char remaining_time[9];
		int percentage_complete;
		char state[10];
} parser_status_info_t;

class CommandParserStream: public StreamOutput {
	public:
		CommandParserStream();
		~CommandParserStream();
		int _putc(int c);
		int _getc(void);
		int puts(const char* str);
		void print_file_list();
		int has_file_list();

		bool file_was_selected();

		bool has_status_info();
		parser_status_info_t get_status_info();

		const char* get_current_filename();
		const char* get_next_filename();
		const char* get_previous_filename();
		unsigned int get_number_of_files();

	private:
		int reading_files = 0;
		vector<std::string> filenames;
		vector<std::string>::iterator fileiterator;

		static parser_status_info_t info;
};

#endif /* COMMANDPARSERSTREAM_H_ */

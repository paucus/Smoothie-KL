#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
// BEGIN MODIF client_print
#include <time.h>
// END MODIF client_print
// BEGIN MODIF public_data
#include "PublicData.h"
// END MODIF public_data
// BEGIN MODIF printf_buff
#include <stdio.h>
// END MODIF printf_buff
// BEGIN MODIF printer_mode
#include <initializer_list>
// END MODIF printer_mode

string lc(const string& str);

bool is_alpha( int );
bool is_digit( int );
bool is_numeric( int );
bool is_alphanum( int );
bool is_whitespace( int );

vector<string> split(const char *str, char c = ',');
vector<float> parse_number_list(const char *str);

string remove_non_number( string str );

uint16_t get_checksum(const string& to_check);
uint16_t get_checksum(const char* to_check);

void get_checksums(uint16_t check_sums[], const string& key);

string shift_parameter( string &parameters );

string get_arguments( string possible_command );

// BEGIN MODIF memory_usage
bool file_exists( const char* file_name );
// END MODIF memory_usage

void system_reset( bool dfu= false );

string absolute_from_relative( string path );

string trim(std::string const& str);
int append_parameters(char *buf, std::vector<std::pair<char,float>> params, size_t bufsize);

// BEGIN MODIF client_print
// This constant is a number that can be used to make format_time display **:**:**.
#define FORMAT_TIME_INFINITE_HOURS 1000000
void convert_to_time_units(time_t total_printing_seconds, long* hours, int* minutes, int* seconds);
// format_time formats hours, minutes and seconds into time. This string will ALWAYS fit in a 9
// characters string (8 for the string, 1 for the '\0'). If it is less than 100 hours, it writes
// HH:MM:SS. If more, it writes H..H:MM (that would be space-left-padded hours up,
// zero-padded-two-digits for minutes) up to 10000 hours. Otherwise, it writes **:**:**.
void format_time(char* dst, int hours, int minutes, int seconds);
const char* trim_left_cstr(const char* l);
const char* move_to_first_space_cstr(const char* l);
long flen(const char* l);
// END MODIF client_print
// BEGIN MODIF second_config_file
// Create a file if it doesn't exists and leaves it empty.
bool ensure_file_exists( const char* file_name );
// END MODIF second_config_file
// BEGIN MODIF public_data
template< typename T> T* get_public_data_ptr(uint16_t csa, uint16_t csb, uint16_t csc, T* default_val) {
	void* ptr;
	bool ok = PublicData::get_value(csa, csb, csc, &ptr);
	if (ok) {
		return (T*)ptr;
	} else {
		return default_val;
	}
};
template< typename T> T get_public_data(uint16_t csa, uint16_t csb, uint16_t csc, T default_val) {
	void* ptr;
	bool ok = PublicData::get_value(csa, csb, csc, &ptr);
	if (ok) {
		return *(T*)ptr;
	} else {
		return default_val;
	}
};
template< typename T> T get_public_data_val(uint16_t csa, uint16_t csb, uint16_t csc, T default_val) {
	T val;
	bool ok = PublicData::get_value(csa, csb, csc, &val);
	if (ok) {
		return val;
	} else {
		return default_val;
	}
};
// END MODIF public_data
// BEGIN MODIF proxy
// Returns the number of digits (in base 10) of the given unsigned int.
unsigned int num_digits(unsigned int i);
// END MODIF proxy
// BEGIN MODIF printf_buff
// Returns the length of the string discarding all printf wildcards including the ending NULL char.
// This constexpr parses the format string at compilation time and returns the string without the
// format wildcards in order to compute the whole string length as printf_fmt_len + sum of item's
// lengths.
// WARNING: Experimental. Use printf_fmt_len with caution. If you are using strange printf features, verify the result is ok.
constexpr bool is_format_char(char c) {
	// http://www.cplusplus.com/reference/cstdio/printf/
	return c=='d'||c=='i'||c=='u'||c=='o'||c=='x'||c=='X'||c=='f'||c=='F'||c=='e'||c=='E'||c=='g'||c=='G'||c=='a'||c=='A'||c=='c'||c=='s'||c=='p'||c=='n';
}
constexpr unsigned int print_fmt_len(const char* str, bool wildcard = 0) {
	return *str?( // There's string to parse
		// If wildcarded, discard all chars until a non-int or dot char is found. It's
		// better to return a longer length than a shorter (to avoid segmentation faults)
		wildcard? (
			*str == '%'?(
				// special case: '%%=%'
				print_fmt_len(str+1, false) + 1
			):(
				// Otherwise, don't count this char and verify if it's the end of the wildcard
				print_fmt_len(str+1, !is_format_char(*str))
			)
		)
		:
		(
			// If not wildcarded, add the current char if not % and analyze if a wildcard starts
			print_fmt_len(str+1, *str=='%') + (*str!='%')
		)
	):(
		// Sanity check: unfinished wildcards
		wildcard?
		//throw assert_failure("Unfinished wildcard")
		(fprintf(stderr, "WARNING: Unfinished wildcard!!!\n") && 0)
		:
		1 // Count the ending NULL char
	); // Null char. End of string.
}
// END MODIF printf_buff
// BEGIN MODIF printer_mode
// Expects two numeric strings and returns the equivalent of doing sign(atoi(n1) - atoi(ns)).
// Any non-numeric character is considered the end of the number. Only unsigned integers are
// handled.
int str_uint_cmp(const char* n1, const char* n2);
// This commands expects a line of code and a valid untrimmed uppercased G/M command as second the
// parameter. Then it compares them, and returns 0 if they are equal (just the G/M code, the rest
// is ignored), -1 if it is "lower" or 1 if it is "greater".
int is_command(const char* line, const char* cmd);

template <typename T>
bool is_in(const T& val, const std::initializer_list<T>& list)
{
    for (const auto& i : list) {
        if (val == i) {
            return true;
        }
    }
    return false;
}
// This function returns il1 if c == true, and il2 if c == false.
// For some reason this can't be done: en?il1:il2. So, this function replaces that.
template <typename T>
const std::initializer_list<T>& cond_il(bool c, const std::initializer_list<T>& il1, const std::initializer_list<T> & il2) {
	if (c){
		return il1;
	} else {
		return il2;
	}
}
// END MODIF printer_mode
#endif

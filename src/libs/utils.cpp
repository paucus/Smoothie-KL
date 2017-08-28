/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Kernel.h"
#include "libs/utils.h"
#include "system_LPC17xx.h"
#include "LPC17xx.h"
#include "utils.h"

#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdlib>
// BEGIN MODIF printer_mode
#include <ctype.h>
// END MODIF printer_mode

using std::string;

uint16_t get_checksum(const string &to_check)
{
    return get_checksum(to_check.c_str());
}

uint16_t get_checksum(const char *to_check)
{
    // From: http://en.wikipedia.org/wiki/Fletcher%27s_checksum
    uint16_t sum1 = 0;
    uint16_t sum2 = 0;
    const char *p = to_check;
    char c;
    while((c = *p++) != 0) {
        sum1 = (sum1 + c) % 255;
        sum2 = (sum2 + sum1) % 255;
    }
    return (sum2 << 8) | sum1;
}

void get_checksums(uint16_t check_sums[], const string &key)
{
    check_sums[0] = 0x0000;
    check_sums[1] = 0x0000;
    check_sums[2] = 0x0000;
    size_t begin_key = 0;
    unsigned int counter = 0;
    while( begin_key < key.size() && counter < 3 ) {
        size_t end_key =  key.find_first_of(".", begin_key);
        string key_node;
        if(end_key == string::npos) {
            key_node = key.substr(begin_key);
        } else {
            key_node = key.substr(begin_key, end_key - begin_key);
        }

        check_sums[counter] = get_checksum(key_node);
        if(end_key == string::npos) break;
        begin_key = end_key + 1;
        counter++;
    }
}

bool is_alpha(int c)
{
    if ((c >= 'a') && (c <= 'z')) return true;
    if ((c >= 'A') && (c <= 'Z')) return true;
    if ((c == '_')) return true;
    return false;
}

bool is_digit(int c)
{
    if ((c >= '0') && (c <= '9')) return true;
    return false;
}

bool is_numeric(int c)
{
    if (is_digit(c)) return true;
    if ((c == '.') || (c == '-')) return true;
    if ((c == 'e')) return true;
    return false;
}

bool is_alphanum(int c)
{
    return is_alpha(c) || is_numeric(c);
}

bool is_whitespace(int c)
{
    if ((c == ' ') || (c == '\t')) return true;
    return false;
}

// Convert to lowercase
string lc(const string &str)
{
    string lcstr;
    for (auto c : str) {
        lcstr.append(1, ::tolower(c));
    }
    return lcstr;
}

// Remove non-number characters
string remove_non_number( string str )
{
    string number_mask = "0123456789-.abcdefpxABCDEFPX";
    size_t found = str.find_first_not_of(number_mask);
    while (found != string::npos) {
        //str[found]='*';
        str.replace(found, 1, "");
        found = str.find_first_not_of(number_mask);
    }
    return str;
}

// Get the first parameter, and remove it from the original string
string shift_parameter( string &parameters )
{
    size_t beginning = parameters.find_first_of(" ");
    if( beginning == string::npos ) {
        string temp = parameters;
        parameters = "";
        return temp;
    }
    string temp = parameters.substr( 0, beginning );
    parameters = parameters.substr(beginning + 1, parameters.size());
    return temp;
}

// Separate command from arguments
string get_arguments( string possible_command )
{
    size_t beginning = possible_command.find_first_of(" ");
    if( beginning == string::npos ) {
        return "";
    }
    return possible_command.substr( beginning + 1, possible_command.size() - beginning + 1);
}

// Returns true if the file exists
// BEGIN MODIF memory_usage
bool file_exists( const char* file_name ){
    bool exists = false;
    FILE *lp = fopen(file_name, "r");
// END MODIF memory_usage
    if(lp) {
        exists = true;
    }
    fclose(lp);
    return exists;
}
// BEGIN MODIF second_config_file
bool ensure_file_exists( const char* file_name ){
	FILE* f = fopen(file_name, "ab+");
	if (f)
		fclose(f);

	return !(!f);
}
// END MODIF second_config_file

// Prepares and executes a watchdog reset for dfu or reboot
void system_reset( bool dfu )
{
    if(dfu) {
        LPC_WDT->WDCLKSEL = 0x1;                // Set CLK src to PCLK
        uint32_t clk = SystemCoreClock / 16;    // WD has a fixed /4 prescaler, PCLK default is /4
        LPC_WDT->WDTC = 1 * (float)clk;         // Reset in 1 second
        LPC_WDT->WDMOD = 0x3;                   // Enabled and Reset
        LPC_WDT->WDFEED = 0xAA;                 // Kick the dog!
        LPC_WDT->WDFEED = 0x55;
    } else {
        NVIC_SystemReset();
    }
}

// Convert a path indication ( absolute or relative ) into a path ( absolute )
// TODO: Combine with plan9 absolute_path, current_path as argument?
string absolute_from_relative( string path )
{
    string cwd = THEKERNEL->current_path;

    if ( path.empty() ) {
        return THEKERNEL->current_path;
    }

    if ( path[0] == '/' ) {
        return path;
    }

    while ( path.substr(0, 3) == "../" ) {
        path = path.substr(3);
        unsigned found = cwd.find_last_of("/");
        cwd = cwd.substr(0, found);
    }

    if ( path.substr(0, 2) == ".." ) {
        path = path.substr(2);
        unsigned found = cwd.find_last_of("/");
        cwd = cwd.substr(0, found);
    }

    if ( cwd[cwd.length() - 1] == '/' ) {
        return cwd + path;
    }

    return cwd + '/' + path;
}

// FIXME this does not handle empty strings correctly
//split a string on a delimiter, return a vector of the split tokens
vector<string> split(const char *str, char c)
{
    vector<string> result;

    do {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

// FIXME this does not handle empty strings correctly
// parse a number list "1.1,2.2,3.3" and return the numbers in a vector of floats
vector<float> parse_number_list(const char *str)
{
    vector<string> l= split(str, ',');
    vector<float> r;
    for(auto& s : l) {
        float x = strtof(s.c_str(), nullptr);
        r.push_back(x);
    }
    return r;
}
string trim(std::string const& str)
{
    std::size_t first = str.find_first_not_of(' ');

    // If there is no non-whitespace character, both first and last will be std::string::npos (-1)
    // There is no point in checking both, since if either doesn't work, the
    // other won't work, either.
    if(first == std::string::npos)
        return "";

    std::size_t last  = str.find_last_not_of(' ');

    return str.substr(first, last-first+1);
}

// BEGIN MODIF client_print
void convert_to_time_units(time_t total_printing_seconds, long* hours, int* minutes, int* seconds) {
    *hours = total_printing_seconds / 3600;
    unsigned int remaining_mins = total_printing_seconds % 3600;
    *minutes = remaining_mins / 60;
    *seconds = remaining_mins % 60;
}
void format_time(char* dst, int hours, int minutes, int seconds) {
	if (hours < 100) {
		// less than 100 hours, write HH:MM:SS
		sprintf(dst, "%02d:%02d:%02d", hours, minutes, seconds);
	} else if (hours < 100000) {
		// more than 100 hours, write HH..H:MM (all the necessary digits for hours)
		sprintf(dst, "% 5d:%02d", hours, minutes);
	} else {
		 // patollocal case when it reports more than 10000 hours
		strcpy(dst, "**:**:**");
	}
}
const char* trim_left_cstr(const char* l){
	while (*l==' ') l++;
	return l;
}
const char* move_to_first_space_cstr(const char* l){
	while (*l!=' '&& *l!='\0') l++;
	return l;
}
long flen(const char* file){
	FILE* f = fopen(file, "rt");
	if (!f){
		return -1;
	}
	// calculate total file size
	fseek(f, 0, SEEK_END);
	long total_file_size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fclose(f);
	return total_file_size;
}
// END MODIF client_print

int append_parameters(char *buf, std::vector<std::pair<char,float>> params, size_t bufsize)
{
    size_t n= 0;
    for(auto &i : params) {
        if(n >= bufsize) break;
        buf[n++]= i.first;
        n += snprintf(&buf[n], bufsize-n, "%1.4f ", i.second);
    }
    return n;
}
// BEGIN MODIF proxy
unsigned int num_digits(unsigned int i) {
    unsigned int length = 0;
    if (i >= 100000000) { i /= 100000000; length += 8; };
    if (i >= 10000) { i /= 10000; length += 4; };
    if (i >= 100) { i /= 100; length += 2; };
    if (i >= 10) { i /= 10; length += 1; };
    return length+1;
}
// END MODIF proxy
// BEGIN MODIF printer_mode
int str_uint_cmp(const char* n1, const char* n2) {
	int result_so_far = 0;

	for (int i = 0; true; i++) {
		char dn1 = n1[i];
		char dn2 = n2[i];
		bool idn1 = isdigit(dn1);
		bool idn2 = isdigit(dn2);

		// We have a hint of a possible bigger number. The only remaining thing is to know
		// which string has more numeric characters.

		if (!idn1 && !idn2) {
			// Both numbers have same length and digits. They are the same.
			return result_so_far;
		} else if (idn1 && idn2) {
			if (result_so_far == 0) {
				// They are both digits. Compare them to see if we find a hint of a possible bigger number.
				// If we have already found a bigger/lower digit before, ignore the current one.
				if (dn1 < dn2) {
					result_so_far = -1;
				} else if (dn1 > dn2) {
					result_so_far = 1;
				} else {
					// same number. No hint yet.
				}
			}
		} else {
			// One of both is not a digit. This means we know which number is shorter.
			return idn1?1:-1;
		}
	}
}
int is_command(const char* line, const char* cmd) {
	// First we compare the first letter. With that we can recognize also empty lines or less
	// common cases.
	if (line[0] != cmd[0]) {
		if (line[0] != 'M' && line[0] != 'G') {
			// It doesn't matter which case it is, it won't ever match, so, we return -1 to stop
			// the comparison loop.
			return -1;
		}
		// We know it is a G/M code, but we know they are different, so, as only G and M codes are
		// handled, we know that only with the G/M character we know if it is greater or lower.
		return line[0] == 'G'?-1:1; // Gxx < Mxx
	}

	// Same G/M character. The rest is a numeric comparison.
	return str_uint_cmp(&(line[1]), &(cmd[1]));
}
// END MODIF printer_mode

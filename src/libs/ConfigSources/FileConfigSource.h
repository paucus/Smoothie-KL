/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FILECONFIGSOURCE_H
#define FILECONFIGSOURCE_H

#include "ConfigSource.h"

class ConfigCache;

using namespace std;
#include <string>
#include <stdio.h>

class FileConfigSource : public ConfigSource
{
public:
	// BEGIN MODIF config_file_not_string
	FileConfigSource(const char* config_file, const char *name);
	// END MODIF config_file_not_string
    void transfer_values_to_cache( ConfigCache *cache );
    void transfer_values_to_cache( ConfigCache *cache, const char * file_name );
    bool is_named( uint16_t check_sum );
    bool write( string setting, string value );
    string read( uint16_t check_sums[3] );
    bool has_config_file();
    void try_config_file(const char* candidate);
    // BEGIN MODIF config_file_not_string
    const char* get_config_file();
    // END MODIF config_file_not_string

private:
    bool readLine(string& line, int lineno, FILE *fp);
    // BEGIN MODIF config_file_not_string
    const char* config_file;         // Path to the config file
    // END MODIF config_file_not_string
    bool   config_file_found;   // Wether or not the config file's location is known
};



#endif

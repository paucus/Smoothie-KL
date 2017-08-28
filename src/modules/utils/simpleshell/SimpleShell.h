/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef simpleshell_h
#define simpleshell_h

#include "Module.h"
// BEGIN MODIF external sdcard
#include "DirHandle.h"
// END MODIF external sdcard

#include <functional>
#include <string>
using std::string;

class StreamOutput;

// BEGIN MODIF fat
enum sort_field {sf_file_size, sf_last_modif_date};
// END MODIF fat

class SimpleShell : public Module
{
public:
    SimpleShell() {}

    void on_module_loaded();
    void on_console_line_received( void *argument );
    void on_gcode_received(void *argument);
    void on_second_tick(void *);
    static bool parse_command(const char *cmd, string args, StreamOutput *stream);
    // BEGIN MODIF external sdcard
    void on_config_reload(void* argument);
    // END MODIF external sdcard

private:
    static void ls_command(string parameters, StreamOutput *stream );
    static void cd_command(string parameters, StreamOutput *stream );
    static void delete_file_command(string parameters, StreamOutput *stream );
    static void pwd_command(string parameters, StreamOutput *stream );
    static void cat_command(string parameters, StreamOutput *stream );
    static void rm_command(string parameters, StreamOutput *stream );
    static void mv_command(string parameters, StreamOutput *stream );
    static void upload_command(string parameters, StreamOutput *stream );
    // BEGIN MODIF update_firmware
    static void cp_command(string parameters, StreamOutput *stream );
    static int copy(const char* from, const char* to, StreamOutput *progress_stream );
    static int copy(const char* from, const char* to, bool update_display);
    static int copy(const char* from, const char* to, bool update_display, StreamOutput *progress_stream );
    static int safe_copy(const char* src, const char* tmp, const char* dst, const char* src_rename, const char* src_desc, const char* dst_desc, StreamOutput *stream);
    static void mkdir_command(string parameters, StreamOutput *stream );
    static void rename_command(string parameters, StreamOutput *stream );
#if SPLIT_CONFIG_AND_PUBLIC_SD
    static void upgrade_command(string parameters, StreamOutput *stream );
#endif // SPLIT_CONFIG_AND_PUBLIC_SD
    // END MODIF update_firmware
    static void break_command(string parameters, StreamOutput *stream );
    static void reset_command(string parameters, StreamOutput *stream );
    static void dfu_command(string parameters, StreamOutput *stream );
    static void help_command(string parameters, StreamOutput *stream );
    static void version_command(string parameters, StreamOutput *stream );
    static void get_command(string parameters, StreamOutput *stream );
    static void set_temp_command(string parameters, StreamOutput *stream );
    static void calc_thermistor_command( string parameters, StreamOutput *stream);
    static void print_thermistors_command( string parameters, StreamOutput *stream);
    static void md5sum_command( string parameters, StreamOutput *stream);

    static void switch_command(string parameters, StreamOutput *stream );
    static void mem_command(string parameters, StreamOutput *stream );

    static void net_command( string parameters, StreamOutput *stream);
    // BEGIN MODIF net_notif
#ifndef NONETWORK
    static void wget_command( string parameters, StreamOutput *stream);
    static void wget_cancel_command( string parameters, StreamOutput *stream);
#endif // NONETWORK
    // END MODIF net_notif
    // BEGIN MODIF dns
#ifndef NONETWORK
    static void nslookup_command( string parameters, StreamOutput *stream);
#endif // NONETWORK
    // END MODIF dns
    // BEGIN MODIF uptime
    static void uptime_command( string parameters, StreamOutput *stream);
    // END MODIF uptime

    static void load_command( string parameters, StreamOutput *stream);
    static void save_command( string parameters, StreamOutput *stream);

    static void remount_command( string parameters, StreamOutput *stream);

    // BEGIN MODIF external sdcard
    static void list_dir_content_sorted(DIR *d, string& folder, enum sort_field sf, bool long_list, bool reverse, bool gcode_only, StreamOutput* stream);
    static void list_dir_content_unsorted(DIR *d, string& folder, bool long_list, bool gcode_only, StreamOutput* stream);
    // END MODIF external sdcard

    typedef void (*PFUNC)(string parameters, StreamOutput *stream);

    typedef struct {
        const char *command;
        const PFUNC func;
    } const ptentry_t;

    static const ptentry_t commands_table[];
    static int reset_delay_secs;
    // BEGIN MODIF external sdcard
    static bool disable_file_sorting;
    static int number_of_files_to_display_when_sorting;
    // END MODIF external sdcard
    // BEGIN MODIF net_notif
    #ifndef NONETWORK
    static volatile bool abort_downloads;
    #endif // NONETWORK
    // END MODIF net_notif
};


#endif

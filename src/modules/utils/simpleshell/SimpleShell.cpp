/*
    This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
    Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/


#include "SimpleShell.h"
#include "libs/Kernel.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"
#include "libs/SerialMessage.h"
#include "libs/StreamOutput.h"
#include "modules/robot/Conveyor.h"
#include "mri.h"
#include "version.h"
#include "PublicDataRequest.h"
#include "AppendFileStream.h"
#include "FileStream.h"
#include "checksumm.h"
#include "PublicData.h"
#include "Gcode.h"
#include "Robot.h"

#include "modules/tools/temperaturecontrol/TemperatureControlPublicAccess.h"
#include "NetworkPublicAccess.h"
#include "platform_memory.h"
#include "SwitchPublicAccess.h"
#include "SDFAT.h"
#include "Thermistor.h"
#include "md5.h"

#include "system_LPC17xx.h"
#include "LPC17xx.h"

#include "mbed.h" // for wait_ms()

extern unsigned int g_maximumHeapAddress;

#include <malloc.h>
#include <mri.h>
#include <stdio.h>
#include <stdint.h>
// BEGIN MODIF fat
#include <sys/stat.h>
#include <time.h>
#include <algorithm>
#include "integer.h"
// END MODIF fat
// BEGIN MODIF external sdcard
#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
// END MODIF external sdcard
#include <errno.h>
#include "LCD4DModule.h"

// BEGIN MODIF external sdcard
#include "path_buffer_const.h"

#define disable_file_sorting_checksum                           CHECKSUM("disable_file_sorting")
#define number_of_files_to_display_when_sorting_checksum        CHECKSUM("number_of_files_to_display_when_sorting")
bool SimpleShell::disable_file_sorting;
int SimpleShell::number_of_files_to_display_when_sorting;
// END MODIF external sdcard
// BEGIN MODIF fat
static const char* IGNORE_DIRS[] = {"System Volume Information", "$RECYCLE.BIN", "__MACOSX", "lost+found", NULL};
// END MODIF fat

extern "C" uint32_t  __end__;
extern "C" uint32_t  __malloc_free_list;
extern "C" uint32_t  _sbrk(int size);

// BEGIN MODIF update_firmware
// every 512KB writen, call on_idle and give feedback to the user in cp and md5sum
#define ON_IDLE_PERIODIC_CALL_BYTE_PERIOD (1 << 19)
#define COPY_BUFF_SIZE 256
#include "md5.h"
#include "LCD4DUpdateProgressScreenScreen.h"
// END MODIF update_firmware
// BEGIN MODIF net_notif
#include "HttpFacade.h"
#include "Uptime.h"
volatile bool SimpleShell::abort_downloads = false;
// END MODIF net_notif
// BEGIN MODIF dns
#include "DnsTaskNsLookup.h"
// END MODIF dns
// BEGIN MODIF remove_file_ext_screen
// This list must be null terminated
const char* GCODE_EXTENSIONS[] = {".gcode", ".g", ".gco", NULL};
// END MODIF remove_file_ext_screen

// command lookup table
const SimpleShell::ptentry_t SimpleShell::commands_table[] = {
    {"ls",       SimpleShell::ls_command},
    {"cd",       SimpleShell::cd_command},
    {"pwd",      SimpleShell::pwd_command},
    {"cat",      SimpleShell::cat_command},
    {"rm",       SimpleShell::rm_command},
    {"mv",       SimpleShell::mv_command},
    {"upload",   SimpleShell::upload_command},
    // BEGIN MODIF update_firmware
    {"cp",       SimpleShell::cp_command},
    {"rename",   SimpleShell::rename_command},
    {"mkdir",    SimpleShell::mkdir_command},
#if SPLIT_CONFIG_AND_PUBLIC_SD
    {"upgrade",       SimpleShell::upgrade_command},
#endif // SPLIT_CONFIG_AND_PUBLIC_SD
    // END MODIF update_firmware
    {"reset",    SimpleShell::reset_command},
    {"dfu",      SimpleShell::dfu_command},
    {"break",    SimpleShell::break_command},
    {"help",     SimpleShell::help_command},
    {"?",        SimpleShell::help_command},
    {"version",  SimpleShell::version_command},
    {"mem",      SimpleShell::mem_command},
    {"get",      SimpleShell::get_command},
    {"set_temp", SimpleShell::set_temp_command},
    {"switch",   SimpleShell::switch_command},
    // BEGIN MODIF net_notif
#ifndef NONETWORK
    {"wget-cancel", SimpleShell::wget_cancel_command},
    {"wget", SimpleShell::wget_command},
#endif //NONETWORK
    // END MODIF net_notif
    // BEGIN MODIF dns
#ifndef NONETWORK
    {"nslookup", SimpleShell::nslookup_command},
#endif // NONETWORK
    // END MODIF dns
    {"net",      SimpleShell::net_command},
    {"load",     SimpleShell::load_command},
    {"save",     SimpleShell::save_command},
    {"remount",  SimpleShell::remount_command},
    {"calc_thermistor", SimpleShell::calc_thermistor_command},
    {"thermistors", SimpleShell::print_thermistors_command},
    {"md5sum",   SimpleShell::md5sum_command},

    // unknown command
    {NULL, NULL}
};

int SimpleShell::reset_delay_secs = 0;

// Adam Greens heap walk from http://mbed.org/forum/mbed/topic/2701/?page=4#comment-22556
static uint32_t heapWalk(StreamOutput *stream, bool verbose)
{
    uint32_t chunkNumber = 1;
    // The __end__ linker symbol points to the beginning of the heap.
    uint32_t chunkCurr = (uint32_t)&__end__;
    // __malloc_free_list is the head pointer to newlib-nano's link list of free chunks.
    uint32_t freeCurr = __malloc_free_list;
    // Calling _sbrk() with 0 reserves no more memory but it returns the current top of heap.
    uint32_t heapEnd = _sbrk(0);
    // accumulate totals
    uint32_t freeSize = 0;
    uint32_t usedSize = 0;

    stream->printf("Used Heap Size: %lu\n", heapEnd - chunkCurr);

    // Walk through the chunks until we hit the end of the heap.
    while (chunkCurr < heapEnd) {
        // Assume the chunk is in use.  Will update later.
        int      isChunkFree = 0;
        // The first 32-bit word in a chunk is the size of the allocation.  newlib-nano over allocates by 8 bytes.
        // 4 bytes for this 32-bit chunk size and another 4 bytes to allow for 8 byte-alignment of returned pointer.
        uint32_t chunkSize = *(uint32_t *)chunkCurr;
        // The start of the next chunk is right after the end of this one.
        uint32_t chunkNext = chunkCurr + chunkSize;

        // The free list is sorted by address.
        // Check to see if we have found the next free chunk in the heap.
        if (chunkCurr == freeCurr) {
            // Chunk is free so flag it as such.
            isChunkFree = 1;
            // The second 32-bit word in a free chunk is a pointer to the next free chunk (again sorted by address).
            freeCurr = *(uint32_t *)(freeCurr + 4);
        }

        // Skip past the 32-bit size field in the chunk header.
        chunkCurr += 4;
        // 8-byte align the data pointer.
        chunkCurr = (chunkCurr + 7) & ~7;
        // newlib-nano over allocates by 8 bytes, 4 bytes for the 32-bit chunk size and another 4 bytes to allow for 8
        // byte-alignment of the returned pointer.
        chunkSize -= 8;
        if (verbose)
            stream->printf("  Chunk: %lu  Address: 0x%08lX  Size: %lu  %s\n", chunkNumber, chunkCurr, chunkSize, isChunkFree ? "CHUNK FREE" : "");

        if (isChunkFree) freeSize += chunkSize;
        else usedSize += chunkSize;

        chunkCurr = chunkNext;
        chunkNumber++;
    }
    stream->printf("Allocated: %lu, Free: %lu\r\n", usedSize, freeSize);
    return freeSize;
}


void SimpleShell::on_module_loaded()
{
    this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_SECOND_TICK);

    reset_delay_secs = 0;
    this->on_config_reload(this);
}

// BEGIN MODIF external sdcard
void SimpleShell::on_config_reload(void* argument){
    disable_file_sorting                    = THEKERNEL->config->value(disable_file_sorting_checksum                    )->by_default(false)->as_bool();
    number_of_files_to_display_when_sorting = THEKERNEL->config->value(number_of_files_to_display_when_sorting_checksum )->by_default(40)->as_number();
}
// END MODIF external sdcard

void SimpleShell::on_second_tick(void *)
{
    // we are timing out for the reset
    if (reset_delay_secs > 0) {
        if (--reset_delay_secs == 0) {
            system_reset(false);
        }
    }
}

static inline bool has_letter_aux(Gcode* gcode, char letter, const char* path_position) {
    return gcode->has_letter(letter) && (!path_position || strchr(gcode->get_command(), letter) < path_position);
}

void SimpleShell::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);
    string args = get_arguments(gcode->get_command());

    if (gcode->has_m) {

        if (gcode->m == 20) { // list sd card
            gcode->stream->printf("Begin file list\r\n");
            // BEGIN MODIF external sdcard
            bool long_list = false;     // -l
            int sort_by_column = 0;     // 1 = -S = file size, 2 = -t = modification time or nothing
            bool reverse = true;        // -r
            bool sort_files = false;    // the negative of -U

            // if P is specified, then we must be cautious that we are not confusing part of the
            // path with arguments.
            const char* path_position = nullptr; // if null, no position was given
            if (gcode->has_letter('P')) {
                // path parameter. Keep the position where it starts.
                path_position = (strchr(gcode->get_command(), 'P') + 1);
            }
            if (has_letter_aux(gcode, 'S', path_position)) {
                sort_files = gcode->get_value('S');
            }
            if (has_letter_aux(gcode, 'L', path_position)) {
                long_list = gcode->get_value('L');
            }
            if (long_list) {
                if (has_letter_aux(gcode, 'C', path_position)) {
                    sort_by_column = gcode->get_value('C');
                }
                if (has_letter_aux(gcode, 'R', path_position)) {
                    reverse = gcode->get_value('R');
                }
            }

            //, extraInfo
            // space characters at the beginning after -G are for -l -U, etc
            char params_char[] = "-G     /" PUBLIC_SD_MOUNT_DIR;
            int i = 2;
            if (!sort_files) params_char[i++] = 'U';
            if (long_list) params_char[i++] = 'l';
            if (reverse) params_char[i++] = 'r';
            if (sort_by_column == 1) params_char[i++] = 'S';
            if (sort_by_column == 2) params_char[i++] = 't';
            string params = string(params_char);

            if (path_position) {
                // append remaining path if any was given
                params += path_position;
            }
            ls_command(params, gcode->stream);
            // END MODIF external sdcard
            gcode->stream->printf("End file list\r\n");
        // END MODIF fat
        } else if (gcode->m == 30) { // remove file
            gcode->stream->printf("'%s'\n", gcode->get_command());
            gcode->stream->printf("'%s'", args.c_str());

            // BEGIN MODIF external sdcard
            rm_command(("/" PUBLIC_SD_MOUNT_DIR "/") + args, gcode->stream);
            // END MODIF external sdcard
        } else if(gcode->m == 501) { // load config override
            // BEGIN MODIF second_config_file
            if(args.empty()) {
                load_command("/" CONFIG_SD_MOUNT_DIR "/config-override", gcode->stream);
            }else{
                load_command("/" CONFIG_SD_MOUNT_DIR "/config-override." + args, gcode->stream);
            }
            // END MODIF second_config_file

        }else if(gcode->m == 504) { // save to specific config override file
            // BEGIN MODIF second_config_file
            if(args.empty()) {
                save_command("/" CONFIG_SD_MOUNT_DIR "/config-override", gcode->stream);
            }else{
                save_command("/" CONFIG_SD_MOUNT_DIR "/config-override." + args, gcode->stream);
            }
            // END MODIF second_config_file
        }
    }
}

bool SimpleShell::parse_command(const char *cmd, string args, StreamOutput *stream)
{
    for (const ptentry_t *p = commands_table; p->command != NULL; ++p) {
        if (strncasecmp(cmd, p->command, strlen(p->command)) == 0) {
            p->func(args, stream);
            return true;
        }
    }

    return false;
}

// When a new line is received, check if it is a command, and if it is, act upon it
void SimpleShell::on_console_line_received( void *argument )
{
    SerialMessage new_message = *static_cast<SerialMessage *>(argument);

    // ignore comments and blank lines and if this is a G code then also ignore it
    char first_char = new_message.message[0];
    if(strchr(";( \n\rGMTN", first_char) != NULL) return;

    string possible_command = new_message.message;

    //new_message.stream->printf("Received %s\r\n", possible_command.c_str());
    string cmd = shift_parameter(possible_command);

    // find command and execute it
    parse_command(cmd.c_str(), possible_command, new_message.stream);
}

// BEGIN MODIF remove_file_ext_screen
static bool has_gcode_compatible_extension(const char* file_name) {
	const char* last_dot = strrchr(file_name, '.');
	if (!last_dot) {
		// It has no extension
		return false;
	}
	const char** gcode_ext = GCODE_EXTENSIONS;
	while (*gcode_ext) {
		if (stricmp(*gcode_ext, last_dot) == 0) {
			return true;
		}
		gcode_ext++;
	}
	return false;
}
// END MODIF remove_file_ext_screen
// BEGIN MODIF fat
static int get_file_info(const string& path, time_t* t, off_t* size, bool* is_dir) {
    struct stat st;
    int result = stat(path.c_str(), &st);
    if (result != 0) {
        *t = 0;
        *size = 0;
        *is_dir = true;	// set to true for the root path dirs
        return result;       // invalid date, return the default date
    }
    *t = st.st_mtime;
    *size = st.st_size;
    *is_dir = (st.st_mode == S_IFDIR);
    return 0;
}

typedef struct dir_entry_sort {
    uint16_t dir_offset;   // 16 bits -> up to 65536 files
    int32_t sort_value;    // 32 bits. Both FAT file size and time_t don't support more than this.
    bool operator!=(struct dir_entry_sort& v) {
        return dir_offset != v.dir_offset || sort_value != v.sort_value;
    }
} dir_entry_sort;

// Overload < operator for the sort algorithm
static bool asc_comparator(const dir_entry_sort s1, const dir_entry_sort s2) {
    return s1.sort_value < s2.sort_value;
}
static bool desc_comparator(const dir_entry_sort s1, const dir_entry_sort s2) {
    return s1.sort_value > s2.sort_value;
}

// FIXME This continuous opendir + closedir is a lot of excess work.
// At first I tried using telldir + seekdir, which would be a better way, however it didn't work.
// Somehow, readdir didn't work after a seekdir. This is a workarround.
// WARNING: If you change this code to use telldir+seekdir, remember to change the type of
// dir_offset from uint16_t to long and adapt the rest of the code (including the corresponding
// format in printfs that print this value).
static DIR* d;
static uint16_t readdir_position;
static void dir_scan_start_dir(const char* path) {
    readdir_position = UINT16_MAX;      // force to read a new dir
    d = NULL;
}
static void dir_scan_close_dir() {
    if (d) {
        closedir(d);
    }
}
// Act upon an ls command
// Convert the first parameter into an absolute path, then list the files in that path
// HEAD version
/*
void SimpleShell::ls_command( string parameters, StreamOutput *stream )
{
    string path, opts;
    while(!parameters.empty()) {
        string s = shift_parameter( parameters );
        if(s.front() == '-') {
            opts.append(s);
        } else {
            path = s;
            if(!parameters.empty()) {
                path.append(" ");
                path.append(parameters);
            }
            break;
        }
    }

    path = absolute_from_relative(path);

    DIR *d;
    struct dirent *p;
    d = opendir(path.c_str());
    if (d != NULL) {
        while ((p = readdir(d)) != NULL) {
            stream->printf("%s", lc(string(p->d_name)).c_str());
            if(p->d_isdir) {
                stream->printf("/");
            } else if(opts.find("-s", 0, 2) != string::npos) {
                stream->printf(" %d", p->d_fsize);
            }
            stream->printf("\r\n");
        }
    } else {
        stream->printf("Could not open directory %s\r\n", path.c_str());
    }
}
*/
static struct dirent *dir_scan_get_dir_position(const char* path, int position) {
    if (readdir_position > position) {
        readdir_position = 0;
        if (d) {
            closedir(d);
        }
        d = opendir(path);
    }
    if (d != NULL) {
        struct dirent *p = readdir(d);
        // read N times
        for(uint16_t i = readdir_position + 1; i <= position; i++) {
            p = readdir(d);
        }
        readdir_position = position + 1;
        return p;
    } else {
        return NULL;
    }
}
static inline string remove_trailing_path_and_generate_string(const char* params_str){
    unsigned int path_length = strlen(params_str);
    if (path_length > 0) {
		const char* end_of_path = &(params_str[strlen(params_str) - 1]);
		if (*end_of_path != '/') {
			end_of_path++;	// ignore last /
		}
		return string(params_str, end_of_path);
    } else {
        return string("");
    }
}
static string parse_parameters(string& params, bool* long_list, bool* sort, bool* reverse, enum sort_field* sort_by_column, bool* gcode_only, StreamOutput *stream) {
    // initialize with default values
    *sort_by_column = sf_last_modif_date;
    *long_list = false;
    *reverse = false;
    *sort = true;
    *gcode_only = false;

    const char* params_str = params.c_str();
    // Ignore trailing white spaces
    while (*params_str == ' ') params_str++;

    while (*params_str == '-'){
        // option parameter
        params_str++;

        do {
            switch (*params_str) {
                case 'l':
                    // long list
                    *long_list = true;
                    break;
                case 'S':
                    // sort by file size
                    *sort_by_column = sf_file_size;
                    break;
                case 't':
                    // sort by modification date
                    *sort_by_column = sf_last_modif_date;
                    break;
                case 'r':
                    *reverse = true;
                    break;
                case 'U':
                    *sort = false;
                    break;
                case 'G':       // this option parameter is a custom one
                    // only show g-code files (*.g*)
                    *gcode_only = true;
                    break;
                case '\0':
                    // String finished!!
                    stream->printf("Missing option parameter.\r\n");
                    return "";
                default:
                    // Invalid parameter
                    stream->printf("Unknown option paramter: -%c\r\n", *params_str);
            }
            params_str++;

        } while (*params_str != ' ' && *params_str != '\0');   // support -ltr syntax

        // Ignore white spaces
        while (*params_str == ' ') params_str++;
    }

    // Now comes the path. If it has a trailing "/", remove it.
    return remove_trailing_path_and_generate_string(params_str);
}

// Act upon an ls command
// Convert the first parameter into an absolute path, then list the files in that path
// Supports -l (detailed list), -S (sort by file size), -t (sort by modification date), -r and -G (show GCode files only)
void SimpleShell::ls_command( string parameters, StreamOutput *stream )
{
    enum sort_field sf;
    bool long_list;
    bool reverse;
    bool sort;
    bool gcode_only;
    string relative_path = parse_parameters(parameters, &long_list, &sort, &reverse, &sf, &gcode_only, stream);

    string folder = absolute_from_relative( relative_path );
    DIR *d = opendir(folder.c_str());
    if (d != NULL) {
        if (sort && !disable_file_sorting) {
            list_dir_content_sorted(d, folder, sf, long_list, reverse, gcode_only, stream);
        } else {
            list_dir_content_unsorted(d, folder, long_list, gcode_only, stream);
        }
    } else {
        stream->printf("Could not open directory %s\r\n", folder.c_str());
    }
}

extern SDFAT mounter;
// BEGIN MODIF external sdcard
#if SPLIT_CONFIG_AND_PUBLIC_SD
extern SDFAT* mounter_sd2;
#endif // SPLIT_CONFIG_AND_PUBLIC_SD
// END MODIF external sdcard

void SimpleShell::remount_command( string parameters, StreamOutput *stream )
{
    mounter.remount();
    // BEGIN MODIF external sdcard
    #if SPLIT_CONFIG_AND_PUBLIC_SD
    if (mounter_sd2)
        mounter_sd2->remount();
    #endif // SPLIT_CONFIG_AND_PUBLIC_SD
    // END MODIF external sdcard
    stream->printf("remounted\r\n");
}

// Delete a file
void SimpleShell::rm_command( string parameters, StreamOutput *stream )
{
    // BEGIN MODIF fat
    const char *fn= absolute_from_relative(parameters ).c_str();        // shift_parameter(
    // END MODIF fat
    int s = remove(fn);
    if (s != 0) stream->printf("Could not delete %s \r\n", fn);
}

static bool is_ignore_dir(const char* name){
    const char** d = IGNORE_DIRS;
    while (*d) {
        if (strcasecmp(name, *d) == 0) {
            return true;
        }
        d++;
    }
    return false;
}

void SimpleShell::list_dir_content_sorted(DIR *d, string& folder, enum sort_field sf, bool long_list, bool reverse, bool gcode_only, StreamOutput* stream) {
    struct dirent *p;
    std::vector<struct dir_entry_sort> dir_entries;

    // First retrieve the list of items. Store the minimum information needed to sort the
    // elements by time or file size later (we have very few RAM space).
    uint16_t current_dir_offset = 0;
    while ((p = readdir(d)) != NULL) {
        if (path_is_too_long(folder, p->d_name)) {
            // File name is too long. It wouldn't fit in some buffers.
            continue;
        }

        // retrieve file info.
        string path = folder + "/" + p->d_name;

        struct dir_entry_sort s;
        s.dir_offset = current_dir_offset;

        off_t file_size;
        time_t t;
        bool is_dir;
        if (get_file_info(path, &t, &file_size, &is_dir) != 0) {
            // do nothing, we will take the default values for them
        }
        s.sort_value = (sf == sf_file_size)?file_size:t;

        if (!gcode_only || (is_dir && !is_ignore_dir(p->d_name)) || has_gcode_compatible_extension(p->d_name)) {
            dir_entries.push_back(s);
            if ((int)dir_entries.size() == number_of_files_to_display_when_sorting) {
                break;
            }
        }
        current_dir_offset++;
    }
    closedir(d);

    // sort elements by the given column
    sort(dir_entries.begin(), dir_entries.end(), reverse?desc_comparator:asc_comparator);

    char str_date[] = "yyyy-mm-dd hh:mm:ss";
    struct tm mdate;
    // Now print the sorted directory entries
    dir_scan_start_dir(folder.c_str());

    for (std::vector<struct dir_entry_sort>::iterator it = dir_entries.begin(); it != dir_entries.end(); ++it) {
        struct dir_entry_sort s = *it;
        // Go to the directory position
        p = dir_scan_get_dir_position(folder.c_str(), s.dir_offset);
        if (p != NULL) {
            // Print detailed information of the files
            string path = folder + "/" + p->d_name;

            off_t file_size;
            time_t t;
            bool is_dir;
            get_file_info(path, &t, &file_size, &is_dir);

            if (long_list) {
                localtime_r(&t, &mdate);
                strftime(str_date, sizeof(str_date), "%F %T", &mdate);
                stream->printf("%c|%s|%lu|%s\r\n", is_dir?'d':'f', p->d_name, file_size, str_date);
            } else {
                stream->printf("%s%s\r\n", p->d_name, is_dir?"/":"");
            }
        } else {
            stream->printf("Failed to retrieve file with position %d.\r\n", s.dir_offset);
        }
    }

    dir_scan_close_dir();
}
void SimpleShell::list_dir_content_unsorted(DIR *d, string& folder, bool long_list, bool gcode_only, StreamOutput* stream) {
    struct dirent *p;
    struct tm mdate;
    off_t file_size;
    bool is_dir;

    char str_date[] = "yyyy-mm-dd hh:mm:ss";
    while ((p = readdir(d)) != NULL) {
        if (path_is_too_long(folder, p->d_name)) {
            // File name is too long. It wouldn't fit in some buffers.
            continue;
        }
        // We only need to get file stats when filtering or printing a list with details
        bool must_get_file_info = gcode_only || long_list;
        if (must_get_file_info) {
            // Print detailed information of the files
            string path = folder + "/" + p->d_name;

            time_t t;
            if (get_file_info(path, &t, &file_size, &is_dir) != 0) {
                // do nothing, we will take the default values for them
            }
            if (long_list) {    // Only make this calculations if these fields will be shown
                localtime_r(&t, &mdate);
                strftime(str_date, sizeof(str_date), "%F %T", &mdate);
            }
        }

        if (!gcode_only || (is_dir && !is_ignore_dir(p->d_name)) || has_gcode_compatible_extension(p->d_name)) {
            if (long_list) {
                stream->printf("%c|%s|%lu|%s\r\n", is_dir?'d':'f', p->d_name, file_size, str_date);
            } else {
                stream->printf("%s%s\r\n", p->d_name, is_dir?"/":"");
            }
        }
    }
    closedir(d);
}

// END MODIF fat
// BEGIN MODIF update_firmware
int SimpleShell::copy(const char* from, const char* to, StreamOutput *progress_stream ) {
    return copy(from, to, false, progress_stream );
}

int SimpleShell::copy(const char* from, const char* to, bool updating_display) {
    return copy(from, to, updating_display, &StreamOutput::NullStream );
}

int SimpleShell::copy(const char* from, const char* to, bool updating_display, StreamOutput* progress_stream) {
    char buff[COPY_BUFF_SIZE];
    FILE* ffrom = fopen(from, "r");
    if (!ffrom)
        return -1;
    FILE* fto = fopen(to, "w");
    if (!fto) {
        fclose(ffrom);
        return -1;
    }
    int count = 0;
    while (!feof(ffrom)) {
        int chread = fread(buff, 1, COPY_BUFF_SIZE, ffrom);
        int chwriten = fwrite(buff, 1, chread, fto);
        if (chread != chwriten) {
            fclose(ffrom);
            fclose(fto);
            return -1;
        }
        count += chread;
        if (count % ON_IDLE_PERIODIC_CALL_BYTE_PERIOD == 0) {    // every 512KB
            progress_stream->printf("copied %d bytes\r\n", count);
            THEKERNEL->call_event(ON_IDLE);
        }
        // force flush after each write. Otherwise, invalid data is written
        // (probably a bug in disk cache).
        fflush(fto);
    }
    progress_stream->printf("copied %d bytes\r\n", count);

    fclose(ffrom);
    fclose(fto);
    return 0;
}
void SimpleShell::cp_command( string parameters, StreamOutput *stream )
{
    string from = absolute_from_relative(shift_parameter( parameters ));
    string to = shift_parameter(parameters);
    int s = SimpleShell::copy(from.c_str(), to.c_str(), stream);
    if (s != 0) stream->printf("Could not copy %s to %s\r\n", from.c_str(), to.c_str());
    else stream->printf("copied %s to %s\r\n", from.c_str(), to.c_str());
}
void SimpleShell::mkdir_command( string parameters, StreamOutput *stream )
{
    string filepath = absolute_from_relative( parameters );
    mkdir(filepath.c_str(), 0);
}
// Rename a file
void SimpleShell::rename_command( string parameters, StreamOutput *stream )
{
    string from = absolute_from_relative(shift_parameter( parameters ));
    string to = shift_parameter(parameters);
    int s = rename(from.c_str(), to.c_str());
    if (s != 0) stream->printf("Could not rename %s to %s\r\n", from.c_str(), to.c_str());
    else stream->printf("renamed %s to %s\r\n", from.c_str(), to.c_str());
}
static bool exists_file(const char* file) {
    FILE* f;
    if ((f = fopen(file, "r")) != NULL) {
        fclose(f);
    }
    return f != NULL;
}
const char* get_name_part(const char* full_path){
    const char* last_slash = full_path;
    while(*full_path){
        if (*full_path=='/' || *full_path=='\\') {
            last_slash = full_path;
        }
        full_path++;
    }
    return last_slash;
}
// BEGIN MODIF md5sum
static void callback_md5(int n) {
    THEKERNEL->call_event(ON_IDLE);
}
static int md5sum(FILE* stream, void* dst){
    // In order to calculate the MD5 of large files, make it periodically call an ON_IDLE event.
    MD5 md5ctx(stream, ON_IDLE_PERIODIC_CALL_BYTE_PERIOD, callback_md5);
    md5ctx.bindigest(dst, 16);
    return 0;
}
// END MODIF md5sum
#ifndef NO_UTILS_LCD
#define gui_print(stream,...) do { stream->printf(__VA_ARGS__); THELCD->get_update_screen()->printf_above_progress(__VA_ARGS__); } while(false)
#else
#define gui_print(stream,...) do { stream->printf(__VA_ARGS__); } while(false)
#endif // NO_UTILS_LCD
int SimpleShell::safe_copy(const char* src, const char* tmp, const char* dst, const char* src_rename, const char* src_desc, const char* dst_desc, StreamOutput* stream) {
    gui_print(stream,"Copying %s to %s!\r\n", src_desc, dst_desc);
    if (SimpleShell::copy(src, tmp, true) != 0) {
        gui_print(stream,"Can't copy %s file to %s.\r\n", src_desc, dst_desc);
        return -1;
    }
    gui_print(stream,"Validating %s file.\r\n", src_desc);
    char md5dig_copy[16];
    char md5dig_orig[16];
    // The firmware doesn't handle opening two files well. So, we will compare
    // MD5s. This can't guarantee with a 100% of confidence that both files are
    // the same, but it will be better for sure.
    FILE* file_to_comp = fopen(src, "r");
    md5sum(file_to_comp, md5dig_orig);
    fclose(file_to_comp);
    file_to_comp = fopen(tmp, "r");
    md5sum(file_to_comp, md5dig_copy);
    fclose(file_to_comp);
    if (memcmp(md5dig_orig, md5dig_copy, sizeof(md5dig_copy)) != 0) {
        gui_print(stream,"Copied image doesn't match original file. Aborting upgrade.\r\n");
        return -1;
    }
    if (exists_file(dst)) {
        // delete any previous image
        int s = remove(dst);
        if (s != 0) {
            gui_print(stream,"Could not delete %s. Aborting upgrade.\r\n", dst);
            return -1;
        }
    }
    gui_print(stream,"Renaming %s so that it's found on next boot.\r\n", src_desc);
    // Rename tmp file to bin file so that it's loaded on next boot
    int s = rename(tmp, dst);
    if (s != 0) {
        gui_print(stream,"Could not rename %s to %s. Aborting process.\r\n", tmp, dst);
        return -1;
    }


    gui_print(stream,"%s in %s card renamed. It will be loaded on next boot.\r\n", src_desc, dst_desc);
    gui_print(stream,"Renaming %s in public SD card.\r\n", src_desc);
    bool continue_process = true;
    if (exists_file(src_rename)) {
        // delete any previous image
        int s = remove(src_rename);
        if (s != 0) {
            gui_print(stream,"Could not delete %s (this is a non-critical error, upgrade will be performed anyway).\r\n", src_rename);
            continue_process = false;
        }
    }
    if (continue_process){
        int s = rename(src, src_rename);
        if (s != 0) {
            gui_print(stream,"Could not rename %s to %s (this is a non-critical error, upgrade will be performed anyway)\r\n", src, src_rename);
        }
    }
    return 0;
}
#if SPLIT_CONFIG_AND_PUBLIC_SD
void SimpleShell::upgrade_command( string parameters, StreamOutput *stream )
{
#ifndef NO_UTILS_LCD
    THELCD->init_upgrade_process();
#endif // NO_UTILS_LCD
    stream->printf("Starting upgrade!\r\n");
    if (!exists_file("/" PUBLIC_SD_MOUNT_DIR "/FIRMWARE.BIN")) {
    	// Internal message
    	stream->printf("No FIRMWARE.BIN file in public SD. Aborting upgrade.\r\n");
        return;
    }

    // now that we have confirmed we are going to perform an upgrade, call the upgrade begin event
    THEKERNEL->call_event(ON_UPGRADE_PROCESS_BEGIN);

    /* We start the LCD update first, so in case it fails, the update will be re attempted automatically.
     * Chances are that only one file failed and we can continue the update without much trouble... */
#ifndef NO_UTILS_LCD
    THELCD->do_update_display();
#endif // NO_UTILS_LCD

    if (exists_file("/" PUBLIC_SD_MOUNT_DIR "/FIRMWARE.BIN"))
        if (safe_copy("/" PUBLIC_SD_MOUNT_DIR "/FIRMWARE.BIN", "/" CONFIG_SD_MOUNT_DIR "/FIRMWARE.TMP", "/" CONFIG_SD_MOUNT_DIR "/FIRMWARE.BIN", "/" PUBLIC_SD_MOUNT_DIR "/FIRMWARE.CUR", "firmware", "internal memory", stream) != 0)
            return;
    if (exists_file("/" PUBLIC_SD_MOUNT_DIR "/KLCONFIG.CNF"))
        if (safe_copy("/" PUBLIC_SD_MOUNT_DIR "/KLCONFIG.CNF", "/" CONFIG_SD_MOUNT_DIR "/KLCONFIG.TMP", "/" CONFIG_SD_MOUNT_DIR "/config", "/" PUBLIC_SD_MOUNT_DIR "/KLCONFIG.CUR", "KLCONFIG.CNF", "internal memory", stream) != 0)
            return;

    gui_print(stream, "Firmware upgrade finished! Restarting printer...\r\n");

    // enqueue a reset command.
    reset_delay_secs = 5;
}
#endif // SPLIT_CONFIG_AND_PUBLIC_SD
// END MODIF update_firmware

// Rename a file
void SimpleShell::mv_command( string parameters, StreamOutput *stream )
{
    string from = absolute_from_relative(shift_parameter( parameters ));
    string to = absolute_from_relative(shift_parameter(parameters));
    int s = rename(from.c_str(), to.c_str());
    if (s != 0) stream->printf("Could not rename %s to %s\r\n", from.c_str(), to.c_str());
    else stream->printf("renamed %s to %s\r\n", from.c_str(), to.c_str());
}

// Change current absolute path to provided path
void SimpleShell::cd_command( string parameters, StreamOutput *stream )
{
    string folder = absolute_from_relative( parameters );

    DIR *d;
    d = opendir(folder.c_str());
    if (d == NULL) {
        stream->printf("Could not open directory %s \r\n", folder.c_str() );
    } else {
        THEKERNEL->current_path = folder;
        closedir(d);
    }
}

// Responds with the present working directory
void SimpleShell::pwd_command( string parameters, StreamOutput *stream )
{
    stream->printf("%s\r\n", THEKERNEL->current_path.c_str());
}

// Output the contents of a file, first parameter is the filename, second is the limit ( in number of lines to output )
void SimpleShell::cat_command( string parameters, StreamOutput *stream )
{
    // Get parameters ( filename and line limit )
    string filename          = absolute_from_relative(shift_parameter( parameters ));
    string limit_paramater   = shift_parameter( parameters );
    int limit = -1;
    if ( limit_paramater != "" ) {
        char *e = NULL;
        limit = strtol(limit_paramater.c_str(), &e, 10);
        if (e <= limit_paramater.c_str())
            limit = -1;
    }

    // Open file
    FILE *lp = fopen(filename.c_str(), "r");
    if (lp == NULL) {
        stream->printf("File not found: %s\r\n", filename.c_str());
        return;
    }
    string buffer;
    int c;
    int newlines = 0;
    int linecnt = 0;
    // Print each line of the file
    while ((c = fgetc (lp)) != EOF) {
        buffer.append((char *)&c, 1);
        if ( char(c) == '\n' || ++linecnt > 80) {
            newlines++;
            stream->puts(buffer.c_str());
            buffer.clear();
            if(linecnt > 80) linecnt = 0;
        }
        if ( newlines == limit ) {
            break;
        }
    };
    fclose(lp);
}

void SimpleShell::upload_command( string parameters, StreamOutput *stream )
{
    // this needs to be a hack. it needs to read direct from serial and not allow on_main_loop run until done
    // NOTE this will block all operation until the upload is complete, so do not do while printing
    if(!THEKERNEL->conveyor->is_queue_empty()) {
        stream->printf("upload not allowed while printing or busy\n");
        return;
    }

    // open file to upload to
    string upload_filename = absolute_from_relative( parameters );
    FILE *fd = fopen(upload_filename.c_str(), "w");
    if(fd != NULL) {
        stream->printf("uploading to file: %s, send control-D or control-Z to finish\r\n", upload_filename.c_str());
    } else {
        stream->printf("failed to open file: %s.\r\n", upload_filename.c_str());
        return;
    }

    int cnt = 0;
    bool uploading = true;
    while(uploading) {
        if(!stream->ready()) {
            // we need to kick things or they die
            THEKERNEL->call_event(ON_IDLE);
            continue;
        }

        char c = stream->_getc();
        if( c == 4 || c == 26) { // ctrl-D or ctrl-Z
            uploading = false;
            // close file
            fclose(fd);
            stream->printf("uploaded %d bytes\n", cnt);
            return;

        } else {
            // write character to file
            cnt++;
            if(fputc(c, fd) != c) {
                // error writing to file
                stream->printf("error writing to file. ignoring all characters until EOF\r\n");
                fclose(fd);
                fd = NULL;
                uploading= false;

            } else {
                if ((cnt%400) == 0) {
                    // HACK ALERT to get around fwrite corruption close and re open for append
                    fclose(fd);
                    fd = fopen(upload_filename.c_str(), "a");
                }
            }
        }
    }
    // we got an error so ignore everything until EOF
    char c;
    do {
        if(stream->ready()) {
            c= stream->_getc();
        }else{
            THEKERNEL->call_event(ON_IDLE);
            c= 0;
        }
    } while(c != 4 && c != 26);
}

// loads the specified config-override file
void SimpleShell::load_command( string parameters, StreamOutput *stream )
{
    // Get parameters ( filename )
    string filename = absolute_from_relative(parameters);
    if(filename == "/") {
        filename = THEKERNEL->config_override_filename();
    }

    FILE *fp = fopen(filename.c_str(), "r");
    if(fp != NULL) {
        char buf[132];
        stream->printf("Loading config override file: %s...\n", filename.c_str());
        while(fgets(buf, sizeof buf, fp) != NULL) {
            stream->printf("  %s", buf);
            if(buf[0] == ';') continue; // skip the comments
            struct SerialMessage message = {&(StreamOutput::NullStream), buf};
            THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message);
        }
        stream->printf("config override file executed\n");
        fclose(fp);

    } else {
        stream->printf("File not found: %s\n", filename.c_str());
    }
}

// saves the specified config-override file
void SimpleShell::save_command( string parameters, StreamOutput *stream )
{
    // Get parameters ( filename )
    string filename = absolute_from_relative(parameters);
    if(filename == "/") {
        filename = THEKERNEL->config_override_filename();
    }

    THEKERNEL->conveyor->wait_for_empty_queue(); //just to be safe as it can take a while to run

    //remove(filename.c_str()); // seems to cause a hang every now and then
    {
        FileStream fs(filename.c_str());
        fs.printf("; DO NOT EDIT THIS FILE\n");
        // this also will truncate the existing file instead of deleting it
    }

    // stream that appends to file
    AppendFileStream *gs = new AppendFileStream(filename.c_str());
    // if(!gs->is_open()) {
    //     stream->printf("Unable to open File %s for write\n", filename.c_str());
    //     return;
    // }

    __disable_irq();
    // issue a M500 which will store values in the file stream
    Gcode *gcode = new Gcode("M500", gs);
    THEKERNEL->call_event(ON_GCODE_RECEIVED, gcode );
    delete gs;
    delete gcode;
    __enable_irq();

    stream->printf("Settings Stored to %s\r\n", filename.c_str());
}

// show free memory
void SimpleShell::mem_command( string parameters, StreamOutput *stream)
{
    bool verbose = shift_parameter( parameters ).find_first_of("Vv") != string::npos ;
    unsigned long heap = (unsigned long)_sbrk(0);
    unsigned long m = g_maximumHeapAddress - heap;
    stream->printf("Unused Heap: %lu bytes\r\n", m);

    uint32_t f = heapWalk(stream, verbose);
    stream->printf("Total Free RAM: %lu bytes\r\n", m + f);

    stream->printf("Free AHB0: %lu, AHB1: %lu\r\n", AHB0.free(), AHB1.free());
    if (verbose) {
        AHB0.debug(stream);
        AHB1.debug(stream);
    }
}

static uint32_t getDeviceType()
{
#define IAP_LOCATION 0x1FFF1FF1
    uint32_t command[1];
    uint32_t result[5];
    typedef void (*IAP)(uint32_t *, uint32_t *);
    IAP iap = (IAP) IAP_LOCATION;

    __disable_irq();

    command[0] = 54;
    iap(command, result);

    __enable_irq();

    return result[1];
}

// get network config
void SimpleShell::net_command( string parameters, StreamOutput *stream)
{
    void *returned_data;
    bool ok = PublicData::get_value( network_checksum, get_ipconfig_checksum, &returned_data );
    if(ok) {
        char *str = (char *)returned_data;
        stream->printf("%s\r\n", str);
        free(str);

    } else {
        stream->printf("No network detected\n");
    }
}
// BEGIN MODIF net_notif
#ifndef NONETWORK
void SimpleShell::wget_command( string parameters, StreamOutput *stream)
{
    static unsigned int total_written = 0;
    static time_t start_time = 0;
    total_written = 0;
    start_time = uptime_millis();
    string p1 = shift_parameter(parameters); // http://www.something.com/file
    string p2 = parameters; // ej: /sd/file.gcode
    if (p2.length() == 0) {
        // parameters not given
        stream->printf("wget <url> <file>\n");
        return;
    }
    stream->printf("downloading %s to %s\n", p1.c_str(), p2.c_str());
    abort_downloads = false;

    // TODO Use tunnel for ssl connections (https), and proxy for the rest
    http_connect_type_t conn_type = httpct_tunnel;

    int result = HttpFacade::instance.download(p1.c_str(), p2.c_str(), [stream](const void* data, uint16_t len) -> tcpprogress_resp_t {
        total_written += len;
        stream->printf("Writing %d bytes (total %d, %1.3f kb/sec)\n", len, total_written, ((float)total_written) / 1.024 / (uptime_millis() - start_time));
        if (abort_downloads) {
            return TCPP_CANCEL;
        } else {
            return TCPP_OK;
        }
    } , [stream]() {
        stream->printf("Finished downloading (%1.3f kb/sec)\n", ((float)total_written) / 1.024 / (uptime_millis() - start_time)); // 1.024 = 1000 msec/sec / 1024 bytes/kb
    }, [stream](int err_num) {
        stream->printf("Error: %s\n", translate_http_err(err_num));
    }, conn_type);
    switch (result) {
    case HTTPFAC_CANNOT_CREATE_FILE:
        stream->printf("Failed to create file %s\n", p2.c_str());
        break;
    case HTTPFAC_INVALID_URL:
        stream->printf("Invalid URL: %s\n", p1.c_str());
        break;
    case HTTPFAC_IOERROR:
        stream->printf("IO Error connecting to URL %s\n", p1.c_str());
        break;
    }
}
void SimpleShell::wget_cancel_command( string parameters, StreamOutput *stream)
{
	abort_downloads = true;
}
#endif // NONETWORK
// END MODIF net_notif
// BEGIN MODIF dns
#ifndef NONETWORK
void SimpleShell::nslookup_command( string parameters, StreamOutput *stream)
{
    const char* host = parameters.c_str();
    // First check in the internal cache.
    u16_t* ip = resolv_lookup(host);
    if (ip && !(!ip[0] && !ip[1])) { // ip != NULL and != 0.0.0.0 (Check index 0 & 1 because they are 16 bits long each)
        // We know the address
        stream->printf("%s: %d.%d.%d.%d (cached)\n", host, uip_ipaddr1(ip), uip_ipaddr2(ip), uip_ipaddr3(ip), uip_ipaddr4(ip));
    } else {
        // Queue a Dns Pending task
        resolv_query(host);
        DnsTaskNsLookup t(host, stream);
        DnsTasks::instance.append_task(&t);
    }
}
#endif // NONETWORK
// END MODIF dns

// print out build version
void SimpleShell::version_command( string parameters, StreamOutput *stream)
{
    Version vers;
    uint32_t dev = getDeviceType();
    const char *mcu = (dev & 0x00100000) ? "LPC1769" : "LPC1768";
    // BEGIN MODIF firmware version
    stream->printf("Version: %s.%s Build version: %s, Build date: %s, MCU: %s, System Clock: %ldMHz\r\n", __VERSION_NUMBER__, __TIMESTAMP_BUILD__, vers.get_build(), vers.get_build_date(), mcu, SystemCoreClock / 1000000);
    // END MODIF firmware version
}

// Reset the system
void SimpleShell::reset_command( string parameters, StreamOutput *stream)
{
    stream->printf("Smoothie out. Peace. Rebooting in 5 seconds...\r\n");
    reset_delay_secs = 5; // reboot in 5 seconds
}

// go into dfu boot mode
void SimpleShell::dfu_command( string parameters, StreamOutput *stream)
{
    stream->printf("Entering boot mode...\r\n");
    system_reset(true);
}

// Break out into the MRI debugging system
void SimpleShell::break_command( string parameters, StreamOutput *stream)
{
    stream->printf("Entering MRI debug mode...\r\n");
    __debugbreak();
}

// used to test out the get public data events
void SimpleShell::get_command( string parameters, StreamOutput *stream)
{
    string what = shift_parameter( parameters );

    if (what == "temp") {
        struct pad_temperature temp;
        string type = shift_parameter( parameters );
        if(type.empty()) {
            // scan all temperature controls
            std::vector<struct pad_temperature> controllers;
            bool ok = PublicData::get_value(temperature_control_checksum, poll_controls_checksum, &controllers);
            if (ok) {
                for (auto &c : controllers) {
                   stream->printf("%s (%d) temp: %f/%f @%d\r\n", c.designator.c_str(), c.id, c.current_temperature, c.target_temperature, c.pwm);
                }

            } else {
                stream->printf("no heaters found\r\n");
            }

        }else{
            bool ok = PublicData::get_value( temperature_control_checksum, current_temperature_checksum, get_checksum(type), &temp );

            if (ok) {
                stream->printf("%s temp: %f/%f @%d\r\n", type.c_str(), temp.current_temperature, temp.target_temperature, temp.pwm);
            } else {
                stream->printf("%s is not a known temperature device\r\n", type.c_str());
            }
        }

    } else if (what == "pos") {
        float pos[3];
        THEKERNEL->robot->get_axis_position(pos);
        stream->printf("Position X: %f, Y: %f, Z: %f\r\n", pos[0], pos[1], pos[2]);
    }
}

// used to test out the get public data events
void SimpleShell::set_temp_command( string parameters, StreamOutput *stream)
{
    string type = shift_parameter( parameters );
    string temp = shift_parameter( parameters );
    float t = temp.empty() ? 0.0 : strtof(temp.c_str(), NULL);
    bool ok = PublicData::set_value( temperature_control_checksum, get_checksum(type), &t );

    if (ok) {
        stream->printf("%s temp set to: %3.1f\r\n", type.c_str(), t);
    } else {
        stream->printf("%s is not a known temperature device\r\n", type.c_str());
    }
}

void SimpleShell::print_thermistors_command( string parameters, StreamOutput *stream)
{
    Thermistor::print_predefined_thermistors(stream);
}

void SimpleShell::calc_thermistor_command( string parameters, StreamOutput *stream)
{
    string s = shift_parameter( parameters );
    int saveto= -1;
    // see if we have -sn as first argument
    if(s.find("-s", 0, 2) != string::npos) {
        // save the results to thermistor n
        saveto= strtol(s.substr(2).c_str(), nullptr, 10);
    }else{
        parameters= s;
    }

    std::vector<float> trl= parse_number_list(parameters.c_str());
    if(trl.size() == 6) {
        // calculate the coefficients
        float c1, c2, c3;
        std::tie(c1, c2, c3) = Thermistor::calculate_steinhart_hart_coefficients(trl[0], trl[1], trl[2], trl[3], trl[4], trl[5]);
        stream->printf("Steinhart Hart coefficients:  I%1.18f J%1.18f K%1.18f\n", c1, c2, c3);
        if(saveto == -1) {
            stream->printf("  Paste the above in the M305 S0 command, then save with M500\n");
        }else{
            char buf[80];
            int n = snprintf(buf, sizeof(buf), "M305 S%d I%1.18f J%1.18f K%1.18f", saveto, c1, c2, c3);
            string g(buf, n);
            Gcode gcode(g, &(StreamOutput::NullStream));
            THEKERNEL->call_event(ON_GCODE_RECEIVED, &gcode );
            stream->printf("  Setting Thermistor %d to those settings, save with M500\n", saveto);
        }

    }else{
        // give help
        stream->printf("Usage: calc_thermistor T1,R1,T2,R2,T3,R3\n");
    }
}

// used to test out the get public data events for switch
void SimpleShell::switch_command( string parameters, StreamOutput *stream)
{
    string type = shift_parameter( parameters );
    string value = shift_parameter( parameters );
    bool ok = false;
    if(value == "on" || value == "off") {
        bool b = value == "on";
        ok = PublicData::set_value( switch_checksum, get_checksum(type), state_checksum, &b );
    } else {
        float v = strtof(value.c_str(), NULL);
        ok = PublicData::set_value( switch_checksum, get_checksum(type), value_checksum, &v );
    }
    if (ok) {
        stream->printf("switch %s set to: %s\r\n", type.c_str(), value.c_str());
    } else {
        stream->printf("%s is not a known switch device\r\n", type.c_str());
    }
}

// END MODIF md5sum
/*
void SimpleShell::md5sum_command( string parameters, StreamOutput *stream )
{
    string filename = absolute_from_relative(parameters);

    // Open file
    FILE *lp = fopen(filename.c_str(), "r");
    if (lp == NULL) {
        stream->printf("File not found: %s\r\n", filename.c_str());
        return;
    }
    MD5 md5;
    uint8_t buf[64];
    do {
        size_t n= fread(buf, 1, sizeof buf, lp);
        if(n > 0) md5.update(buf, n);
    } while(!feof(lp));

    stream->printf("%s %s\n", md5.finalize().hexdigest().c_str(), filename.c_str());
    fclose(lp);
}
*/
void SimpleShell::md5sum_command( string parameters, StreamOutput *stream )
{
    string filepath = absolute_from_relative( parameters );
    FILE* file = fopen(filepath.c_str(), "r");
    if (!file) {
        stream->printf("Failed to open file %s\n", filepath.c_str());
        return;
    }
    char md5dig[16];    //128bits for md5
    int res = md5sum(file, md5dig);
    if (res != 0) {
        stream->printf("Failed to calculate md5 from file %s\n", filepath.c_str());
    } else {
        stream->printf("MD5(%s): %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n", filepath.c_str(), md5dig[0], md5dig[1], md5dig[2], md5dig[3], md5dig[4], md5dig[5], md5dig[6], md5dig[7], md5dig[8], md5dig[9], md5dig[10], md5dig[11], md5dig[12], md5dig[13], md5dig[14], md5dig[15]);
    }
    fclose(file);
}
// END MODIF md5sum



void SimpleShell::help_command( string parameters, StreamOutput *stream )
{
    stream->printf("Commands:\r\n");
    stream->printf("version\r\n");
    stream->printf("mem [-v]\r\n");
    // BEGIN MODIF fat
    stream->printf("ls [-lStrGU] [folder]\r\n");
    // END MODIF fat
    stream->printf("cd folder\r\n");
    stream->printf("pwd\r\n");
    stream->printf("cat file [limit]\r\n");
    stream->printf("rm file\r\n");
    stream->printf("mv file newfile\r\n");
    stream->printf("remount\r\n");
    // BEGIN MODIF update_firmware
    stream->printf("rename file newname\r\n");
    stream->printf("cp file1 file2\r\n");
    stream->printf("md5sum file\r\n");
#if SPLIT_CONFIG_AND_PUBLIC_SD
    stream->printf("upgrade\r\n");
#endif // SPLIT_CONFIG_AND_PUBLIC_SD
    // END MODIF update_firmware
    stream->printf("play file [-v]\r\n");
    stream->printf("progress - shows progress of current play\r\n");
    stream->printf("abort - abort currently playing file\r\n");
    stream->printf("reset - reset smoothie\r\n");
    stream->printf("dfu - enter dfu boot loader\r\n");
    stream->printf("break - break into debugger\r\n");
    stream->printf("config-get [<configuration_source>] <configuration_setting>\r\n");
    stream->printf("config-set [<configuration_source>] <configuration_setting> <value>\r\n");
    stream->printf("get temp [bed|hotend]\r\n");
    stream->printf("set_temp bed|hotend 185\r\n");
    stream->printf("get pos\r\n");
    stream->printf("net\r\n");
    stream->printf("load [file] - loads a configuration override file from soecified name or config-override\r\n");
    stream->printf("save [file] - saves a configuration override file as specified filename or as config-override\r\n");
    stream->printf("upload filename - saves a stream of text to the named file\r\n");
    stream->printf("calc_thermistor [-s0] T1,R1,T2,R2,T3,R3 - calculate the Steinhart Hart coefficients for a thermistor\r\n");
    stream->printf("thermistors - print out the predefined thermistors\r\n");
    stream->printf("md5sum file - prints md5 sum of the given file\r\n");
    // BEGIN MODIF net_notif
    stream->printf("wget url file - downloads the file in the given url to the given file\r\n");
    stream->printf("wget-cancel - cancels all downloads being performed at that moment with wget command\r\n");
    // END MODIF net_notif
}


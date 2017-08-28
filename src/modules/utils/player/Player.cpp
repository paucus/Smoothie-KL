/*
    This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
    Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
    Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Player.h"

#include "libs/Kernel.h"
#include "Robot.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"
#include "SerialConsole.h"
#include "libs/SerialMessage.h"
#include "libs/StreamOutputPool.h"
#include "libs/StreamOutput.h"
#include "Gcode.h"
#include "checksumm.h"
#include "Pauser.h"
#include "Config.h"
#include "ConfigValue.h"
#include "SDFAT.h"

#include "modules/robot/Conveyor.h"
#include "DirHandle.h"
#include "PublicDataRequest.h"
#include "PublicData.h"
#include "PlayerPublicAccess.h"
#include "TemperatureControlPublicAccess.h"
#include "TemperatureControlPool.h"
#include "ExtruderPublicAccess.h"
// BEGIN MODIF time_remaining_calculation
#include "PrecalculationEstimation.h"
#include "HistoricalInformationEstimation.h"
#define estimator_checksum                         CHECKSUM("estimator")
// END MODIF time_remaining_calculation

#include <cstddef>
#include <cmath>
#include <algorithm>

#include "mbed.h"

#define on_boot_gcode_checksum            CHECKSUM("on_boot_gcode")
#define on_boot_gcode_enable_checksum     CHECKSUM("on_boot_gcode_enable")
#define after_suspend_gcode_checksum      CHECKSUM("after_suspend_gcode")
#define before_resume_gcode_checksum      CHECKSUM("before_resume_gcode")
#define leave_heaters_on_suspend_checksum CHECKSUM("leave_heaters_on_suspend")

extern SDFAT mounter;

// BEGIN MODIF ignore_home_time
#include "Uptime.h"
#include "modules/tools/autolevel/AutoLevelStatus.h"
// END MODIF ignore_home_time
// BEGIN MODIF restore_state
//#define M24_gcode_on_resume_checksum                       CHECKSUM("M24_gcode_on_resume")
//#define M25_gcode_on_pause_checksum                        CHECKSUM("M25_gcode_on_pause")
//#define M26_gcode_on_kill_checksum                         CHECKSUM("M26_gcode_on_kill")
#include "GcodeUtils.h"
#include "RobotConsts.h"

// BEGIN MODIF printer_mode
Player* Player::player = nullptr;
// end MODIF printer_mode

//#define DEFAULT_M24_GCODE_ON_RESUME ""
//#define DEFAULT_M25_GCODE_ON_PAUSE "M400;G791;M400;G1 Z30 F" Z_MAX_SPEED_STR ";M400;G790;M400;G1 X0 Y0 F7000;M400"
//#define DEFAULT_M26_GCODE_ON_KILL "G28 X0;M140 S0;M104 S0;M400;M84"
const char* ARRAY_M24_GCODE_ON_RESUME[] = {nullptr};
const char* ARRAY_M25_GCODE_ON_PAUSE[] = {"M400", "G791", "M400", "G1 Z30 F" Z_MAX_SPEED_STR, "M400", "G790", "M400", "G1 X0 Y0 F7000", "M400", nullptr};
const char* ARRAY_M26_GCODE_ON_KILL[] = {"G28 X0", "M140 S0", "M104 S0", "M400", "M84", nullptr};
// END MODIF restore_state

// BEGIN MODIF restore_state
Player::Player() : pause_state(PrinterStateSnapshot::null_state) {
// END MODIF restore_state
    // BEGIN MODIF time_remaining_calculation
    // Just in case someone runs a method that depend on the estimation, start with a NullEstimation.
    estimator = new NullEstimation();
    // END MODIF time_remaining_calculation
    this->playing_file = false;
    this->current_file_handler = nullptr;
    this->booted = false;
    this->elapsed_secs = 0;
    this->reply_stream = nullptr;
    // BEGIN MODIF printer_mode
    Player::player = this;
    // end MODIF printer_mode
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
    this->suspended= false;
    this->suspend_loops= 0;
*/
// END MODIF restore_state
}

void Player::on_module_loaded()
{
    this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
    this->register_for_event(ON_MAIN_LOOP);
    this->register_for_event(ON_SECOND_TICK);
    this->register_for_event(ON_GET_PUBLIC_DATA);
    this->register_for_event(ON_SET_PUBLIC_DATA);
    this->register_for_event(ON_GCODE_RECEIVED);

    this->elapsed_secs = 0;
    
    // BEGIN MODIF ignore_home_time
    this->ignored_secs = 0;
    this->register_for_event(ON_AUTOLEVEL_STATUS_CHANGE);
    // END MODIF ignore_home_time
    this->reply_stream = NULL;
    // BEGIN MODIF restore_state
    // BEGIN MODIF second_config_file
    this->on_boot_gcode = THEKERNEL->config->value(on_boot_gcode_checksum)->by_default("/" CONFIG_SD_MOUNT_DIR "/on_boot.gcode")->as_string();
    // END MODIF second_config_file
    // BEGIN MODIF on_boot_without_end_event
    this->on_boot_gcode_running = false;
    // END MODIF on_boot_without_end_event

    this->on_boot_gcode_enable = THEKERNEL->config->value(on_boot_gcode_enable_checksum)->by_default(true)->as_bool();
    on_config(this);
    // END MODIF restore_state
    
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
// Don't load this config to save RAM
/*
    this->after_suspend_gcode = THEKERNEL->config->value(after_suspend_gcode_checksum)->by_default("")->as_string();
    this->before_resume_gcode = THEKERNEL->config->value(before_resume_gcode_checksum)->by_default("")->as_string();
    std::replace( this->after_suspend_gcode.begin(), this->after_suspend_gcode.end(), '_', ' '); // replace _ with space
    std::replace( this->before_resume_gcode.begin(), this->before_resume_gcode.end(), '_', ' '); // replace _ with space
    this->leave_heaters_on = THEKERNEL->config->value(leave_heaters_on_suspend_checksum)->by_default(false)->as_bool();
*/
// END MODIF restore_state
}

// BEGIN MODIF restore_state
void Player::on_config(void* argument) {
//    extract_gcode_from_string(this->M24_gcode_on_resume, THEKERNEL->config->value( M24_gcode_on_resume_checksum )->by_default(DEFAULT_M24_GCODE_ON_RESUME)->as_string().c_str());
//    extract_gcode_from_string(this->M25_gcode_on_pause, THEKERNEL->config->value( M25_gcode_on_pause_checksum )->by_default(DEFAULT_M25_GCODE_ON_PAUSE)->as_string().c_str());
//    extract_gcode_from_string(this->M26_gcode_on_kill, THEKERNEL->config->value( M26_gcode_on_kill_checksum )->by_default(DEFAULT_M26_GCODE_ON_KILL)->as_string().c_str());

    // BEGIN MODIF time_remaining_calculation
    delete estimator;
    if (THEKERNEL->config->value( player_checksum, estimator_checksum )->by_default("precalculate")->as_string() == "historical") {
        estimator = new HistoricalInformationEstimation();
    } else {
        estimator = new PrecalculationEstimation();
    }
    // END MODIF time_remaining_calculation
}
// END MODIF restore_state

void Player::on_second_tick(void *)
{
    if(this->playing_file) this->elapsed_secs++;
}

// extract any options found on line, terminates args at the space before the first option (-v)
// eg this is a file.gcode -v
//    will return -v and set args to this is a file.gcode
string Player::extract_options(string& args)
{
    string opts;
    size_t pos= args.find(" -");
    if(pos != string::npos) {
        opts= args.substr(pos);
        args= args.substr(0, pos);
    }

    return opts;
}
void Player::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);
    string args = get_arguments(gcode->get_command());
    if (gcode->has_m) {
        if (gcode->m == 21) { // Dummy code; makes Octoprint happy -- supposed to initialize SD card
            mounter.remount();
            gcode->stream->printf("SD card ok\r\n");

        } else if (gcode->m == 23) { // select file
            // BEGIN MODIF external sdcard
            this->filename = ("/" PUBLIC_SD_MOUNT_DIR "/") + args;      //shift_parameter( 
            // END MODIF external sdcard

            this->current_stream = &(StreamOutput::NullStream);

            if(this->current_file_handler != NULL) {
                this->playing_file = false;
                fclose(this->current_file_handler);
            }
            // BEGIN MODIF time_remaining_calculation
            estimator->preprocess_file(this->filename.c_str());
            // END MODIF time_remaining_calculation
            this->current_file_handler = fopen( this->filename.c_str(), "r");

            // Clear pause state snapshot if set (any previous state stored would no longer be valid).
            // We don't want an M24 to attempt to restore any previous state.
            this->pause_state.invalidate();
            // END MODIF restore_state
            // END MODIF end_of_filament

            if(this->current_file_handler == NULL) {
                gcode->stream->printf("file.open failed: %s\r\n", this->filename.c_str());
                return;

            } else {
                // get size of file
                int result = fseek(this->current_file_handler, 0, SEEK_END);
                if (0 != result) {
                    this->file_size = 0;
                } else {
                    this->file_size = ftell(this->current_file_handler);
                    fseek(this->current_file_handler, 0, SEEK_SET);
                }
                gcode->stream->printf("File opened:%s Size:%ld\r\n", this->filename.c_str(), this->file_size);
                gcode->stream->printf("File selected\r\n");
            }


            this->played_cnt = 0;
            this->elapsed_secs = 0;
            // BEGIN MODIF ignore_home_time
            this->ignored_secs = 0;
            // END MODIF ignore_home_time

        } else if (gcode->m == 24) { // start print

            // BEGIN MODIF m24_blockable
            THEKERNEL->conveyor->wait_for_empty_queue();
            // END MODIF m24_blockable

            if (this->current_file_handler != NULL) {
                this->playing_file = true;
                // this would be a problem if the stream goes away before the file has finished,
                // so we attach it to the kernel stream, however network connections from pronterface
                // do not connect to the kernel streams so won't see this FIXME
                this->reply_stream = THEKERNEL->streams;
            }

            // BEGIN MODIF restore_state

            if (this->pause_state.is_valid()) {
                // In this case we are resuming a print, not starting a new one
//                send_all_gcodes(M24_gcode_on_resume, gcode->stream);
                send_all_gcodes(ARRAY_M24_GCODE_ON_RESUME, gcode->stream);
                if ((!gcode->has_letter('F')) || gcode->get_value('F') == 0){    // verify it's not coming from an end of filament event
                    rise_print_resume_event();
                }

                // Restore state if requested
                this->pause_state.restore(gcode->stream);
            } else {
                // gcode begin
                rise_print_begin_event();
            }
            // END MODIF restore_state

        } else if (gcode->m == 25) { // pause print
            this->playing_file = false;

            // BEGIN MODIF restore_state
            this->pause_state = PrinterStateSnapshot::capture();
//            send_all_gcodes(M25_gcode_on_pause, gcode->stream);
            send_all_gcodes(ARRAY_M25_GCODE_ON_PAUSE, gcode->stream);

            if ((!gcode->has_letter('F')) || gcode->get_value('F') == 0){    // verify it's not coming from an end of filament event
                rise_print_pause_event();
            }
            // END MODIF restore_state

        } else if (gcode->m == 26) { // Reset print. Slightly different than M26 in Marlin and the rest
            if(this->current_file_handler != NULL) {
                string currentfn = this->filename.c_str();
                unsigned long old_size = this->file_size;

                // abort the print
                abort_command("", gcode->stream);

                if(!currentfn.empty()) {
                    // reload the last file opened
                    this->current_file_handler = fopen(currentfn.c_str() , "r");

                    if(this->current_file_handler == NULL) {
                        gcode->stream->printf("file.open failed: %s\r\n", currentfn.c_str());
                    } else {
                        this->filename = currentfn;
                        this->file_size = old_size;
                        this->current_stream = &(StreamOutput::NullStream);
                    }
                }
            } else {
                gcode->stream->printf("No file loaded\r\n");
            }

        } else if (gcode->m == 27) { // report print progress, in format used by Marlin
            // BEGIN MODIF client_print
            if (gcode->has_letter('L') && gcode->get_value('L') != 0) {
                progress_command("-x", gcode->stream);
            } else {
                progress_command("-b", gcode->stream);
            }
            // END MODIF client_print

        } else if (gcode->m == 32) { // select file and start print
            // Get filename
            // BEGIN MODIF external sdcard
            this->filename = ("/" PUBLIC_SD_MOUNT_DIR "/") + args ;     // shift_parameter( 
            // END MODIF external sdcard
            this->current_stream = &(StreamOutput::NullStream);

            if(this->current_file_handler != NULL) {
                this->playing_file = false;
                fclose(this->current_file_handler);
            }

            // BEGIN MODIF time_remaining_calculation
            estimator->preprocess_file(this->filename.c_str());
            // END MODIF time_remaining_calculation
            this->current_file_handler = fopen( this->filename.c_str(), "r");
            if(this->current_file_handler == NULL) {
                gcode->stream->printf("file.open failed: %s\r\n", this->filename.c_str());
            } else {
                this->playing_file = true;

                // get size of file
                int result = fseek(this->current_file_handler, 0, SEEK_END);
                if (0 != result) {
                        file_size = 0;
                } else {
                        file_size = ftell(this->current_file_handler);
                        fseek(this->current_file_handler, 0, SEEK_SET);
                }
            }

            this->played_cnt = 0;
            this->elapsed_secs = 0;

            // BEGIN MODIF restore_state
            // We don't want an M24 to attempt to restore any previous state
            this->pause_state.invalidate();
            rise_print_begin_event();
            // END MODIF restore_state

// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
        } else if (gcode->m == 600) { // suspend print, Not entirely Marlin compliant
            this->suspend_command("", gcode->stream);

        } else if (gcode->m == 601) { // resume print
            this->resume_command("", gcode->stream);
*/
// END MODIF restore_state
        }
    }
}

// BEGIN MODIF restore_state
//client_printing_state_t Player::get_current_status() {
//    if(current_file_handler == NULL) {
//        return cp_idle;
//    } else {
//        return playing_file?cp_printing:cp_paused; AHORA QUE HAY EOF, NO ESTOY SEGURO SI LO IDENTIFICO BIEN AL STATE. SINO GUARDALO EN ALGUN LUGAR
//    }
//}
void Player::rise_print_begin_event() {
    print_status_change_t stat;
//    stat.old_status = get_current_status();
    stat.new_status = cp_printing;
    stat.print_source = PS_SD_CARD;
    stat.event = pe_begin;
    stat.event_was_estimated = false;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void Player::rise_print_end_event() {
    print_status_change_t stat;
//    stat.old_status = get_current_status();
    stat.new_status = cp_idle;
    stat.print_source = PS_SD_CARD;
    stat.event = pe_end;
    stat.event_was_estimated = false;
    stat.total_print_time = this->elapsed_secs;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void Player::rise_print_resume_event() {
    print_status_change_t stat;
//    stat.old_status = get_current_status();
    stat.new_status = cp_printing;
    stat.print_source = PS_SD_CARD;
    stat.event = pe_resume;
    stat.event_was_estimated = false;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void Player::rise_print_pause_event() {
    print_status_change_t stat;
//    stat.old_status = get_current_status();
    stat.new_status = cp_paused;
    stat.print_source = PS_SD_CARD;
    stat.event = pe_pause;
    stat.event_was_estimated = false;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
// END MODIF restore_state

// When a new line is received, check if it is a command, and if it is, act upon it
void Player::on_console_line_received( void *argument )
{
    if(THEKERNEL->is_halted()) return; // if in halted state ignore any commands

    SerialMessage new_message = *static_cast<SerialMessage *>(argument);

    // ignore comments and blank lines and if this is a G code then also ignore it
    char first_char = new_message.message[0];
    if(strchr(";( \n\rGMTN", first_char) != NULL) return;

    string possible_command = new_message.message;
    string cmd = shift_parameter(possible_command);

    //new_message.stream->printf("Received %s\r\n", possible_command.c_str());

    // Act depending on command
    if (cmd == "play"){
        this->play_command( possible_command, new_message.stream );
    }else if (cmd == "progress"){
        this->progress_command( possible_command, new_message.stream );
    }else if (cmd == "abort") {
        this->abort_command( possible_command, new_message.stream );
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
    }else if (cmd == "suspend") {
        this->suspend_command( possible_command, new_message.stream );
    }else if (cmd == "resume") {
        this->resume_command( possible_command, new_message.stream );
*/
// END MODIF restore_state
    }
}

// Play a gcode file by considering each line as if it was received on the serial console
void Player::play_command( string parameters, StreamOutput *stream )
// BEGIN MODIF on_boot_without_end_event
{
	play_command( parameters, stream, true);
}
void Player::play_command( string& parameters, StreamOutput *stream , bool was_played_by_user)
// BEGIN MODIF on_boot_without_end_event
{
    // extract any options from the line and terminate the line there
    string options= extract_options(parameters);
    // Get filename which is the entire parameter line upto any options found or entire line
    this->filename = absolute_from_relative(parameters);

// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
// if(this->playing_file || this->suspended) {
    if(this->playing_file) {
// END MODIF restore_state
        stream->printf("Currently printing, abort print first\r\n");
        return;
    }

    if(this->current_file_handler != NULL) { // must have been a paused print
        fclose(this->current_file_handler);
    }

    // BEGIN MODIF time_remaining_calculation
    estimator->preprocess_file(this->filename.c_str());
    // END MODIF time_remaining_calculation
    this->current_file_handler = fopen( this->filename.c_str(), "r");
    if(this->current_file_handler == NULL) {
        stream->printf("File not found: %s\r\n", this->filename.c_str());
        return;
    }

    stream->printf("Playing %s\r\n", this->filename.c_str());

    this->playing_file = true;

    // Output to the current stream if we were passed the -v ( verbose ) option
    if( options.find_first_of("Vv") == string::npos ) {
        this->current_stream = &(StreamOutput::NullStream);
    } else {
        // we send to the kernels stream as it cannot go away
        this->current_stream = THEKERNEL->streams;
    }

    // get size of file
    int result = fseek(this->current_file_handler, 0, SEEK_END);
    if (0 != result) {
        stream->printf("WARNING - Could not get file size\r\n");
        file_size = 0;
    } else {
        file_size = ftell(this->current_file_handler);
        fseek(this->current_file_handler, 0, SEEK_SET);
        stream->printf("  File size %ld\r\n", file_size);
    }
    this->played_cnt = 0;
    this->elapsed_secs = 0;

    // BEGIN MODIF restore_state
    // gcode begin. We might not want to rise the print begin event (on_boot.gcode, for example)
    if (was_played_by_user)
        rise_print_begin_event();
    // END MODIF restore_state
}
// BEGIN MODIF time_remaining_calculation
static float get_speed_override(){
	return 60.0F / THEKERNEL->robot->get_seconds_per_minute();
}
unsigned long Player::calculate_remaining_time(){
    float extrusion_multiplier = get_speed_override();
    return estimator->calculate_remaining_time(extrusion_multiplier);
}
// END MODIF time_remaining_calculation
void Player::progress_command( string parameters, StreamOutput *stream )
{

    // get options
    string options = shift_parameter( parameters );
    bool sdprinting= options.find_first_of("Bb") != string::npos;
    // BEGIN MODIF client_print
    bool extended_format = options.find_first_of("Xx") != string::npos;

    if (extended_format) {
        // If -x or -X is passed, report in the extended format
        if(current_file_handler == NULL) {
            stream->printf("00:00:00|00:00:00|0|IDLE\r\n");
        } else {
            time_t total_printing_seconds;
            unsigned long est;
            long hours;
            int minutes;
            int seconds;
            long est_hours;
            int est_minutes;
            int est_seconds;
            unsigned int pcnt;
            total_printing_seconds = (time_t)this->elapsed_secs;
            convert_to_time_units(total_printing_seconds, &hours, &minutes, &seconds);
            est = 0;
            if(file_size > 0 && this->elapsed_secs > 10) {
                // BEGIN MODIF time_remaining_calculation
                est = calculate_remaining_time();
                if(est != INFINITE_TIME) {
                    // BEGIN MODIF time_remaining_calculation
                    convert_to_time_units(est, &est_hours, &est_minutes, &est_seconds);
                } else {
                    // We can't calculate. Set "infinite" so that **:**:** is displayed.
                    est_hours = FORMAT_TIME_INFINITE_HOURS;
                }
                // END MODIF time_remaining_calculation
            }
            if(file_size > 0) {
                // BEGIN MODIF time_remaining_calculation
                // pcnt = (this->file_size - (this->file_size - this->played_cnt)) * 100 / this->file_size;
                unsigned long c = this->played_cnt;
                unsigned long s = this->file_size;
                if (s >= ~((unsigned long)0)/101){
                    // If the file size is too big, we could have overflow problems. Reduce the resolution.
                    c >>= 8;
                    s >>= 8;
                }
                pcnt = (c * 100 + s / 2)/ s;
                // END MODIF time_remaining_calculation
            } else {
                pcnt = 0;
            }
            char time1_str[] = "00:00:00";
            char time2_str[] = "00:00:00";
            format_time(time1_str, hours, minutes, seconds);
            format_time(time2_str, est_hours, est_minutes, est_seconds);
            stream->printf("%s|%s|%u|%s\r\n", time1_str, time2_str, pcnt, playing_file?"PRINT":"PAUSE");
        }
    } else {
    // END MODIF client_print
        if(!playing_file && current_file_handler != NULL) {
            if(sdprinting)
                stream->printf("SD printing byte %lu/%lu\r\n", played_cnt, file_size);
            else
                stream->printf("SD print is paused at %lu/%lu\r\n", played_cnt, file_size);
            return;

        } else if(!playing_file) {
            stream->printf("Not currently playing\r\n");
            return;
        }

        if(file_size > 0) {
            unsigned long est = 0;
            if(this->elapsed_secs > 10) {
                // BEGIN MODIF time_remaining_calculation
                est = calculate_remaining_time();
                if (est == INFINITE_TIME) {
                    est = 0;    // unknown. Set 0, it is better than an huge arbitrary number.
                }
                // END MODIF time_remaining_calculation
            }

            // BEGIN MODIF time_remaining_calculation
            // unsigned int pcnt = (this->file_size - (this->file_size - this->played_cnt)) * 100 / this->file_size;
            unsigned long c = this->played_cnt;
            unsigned long s = this->file_size;
            if (s >= ~((unsigned long)0)/101){
                // If the file size is too big, we could have overflow problems. Reduce the resolution.
                c >>= 8;
                s >>= 8;
            }
            unsigned int pcnt = (c * 100 + s / 2)/ s;
            // END MODIF time_remaining_calculation
            // If -b or -B is passed, report in the format used by Marlin and the others.
            if (!sdprinting) {
                stream->printf("%u %% complete, elapsed time: %lu s", pcnt, this->elapsed_secs);
                if(est > 0) {
                    stream->printf(", est time: %lu s",  est);
                }
                stream->printf("\r\n");
            } else {
                stream->printf("SD printing byte %lu/%lu\r\n", played_cnt, file_size);
            }

        } else {
            stream->printf("File size is unknown\r\n");
        }
    // BEGIN MODIF client_print
    }
    // END MODIF client_print
}

void Player::abort_command( string parameters, StreamOutput *stream )
{
    if(!playing_file && current_file_handler == NULL) {
        stream->printf("Not currently playing\r\n");
        return;
    }
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
    suspended= false;
*/
// END MODIF restore_state
    playing_file = false;
    played_cnt = 0;
    file_size = 0;
    this->filename = "";
    this->current_stream = NULL;
    fclose(current_file_handler);
    current_file_handler = NULL;
    if(parameters.empty()) {
        // clear out the block queue, will wait until queue is empty
        // MUST be called in on_main_loop to make sure there are no blocked main loops waiting to put something on the queue
        THEKERNEL->conveyor->flush_queue();

        // now the position will think it is at the last received pos, so we need to do FK to get the actuator position and reset the current position
        THEKERNEL->robot->reset_position_from_current_actuator_position();
    }
    stream->printf("Aborted playing or paused file. Please turn any heaters off manually\r\n");
    // BEGIN MODIF restore_state
    // Run ending gcode
//    send_all_gcodes(M26_gcode_on_kill, stream);
    send_all_gcodes(ARRAY_M26_GCODE_ON_KILL, stream);
    rise_print_end_event();
    // Clear pause state snapshot if set.
    this->pause_state.invalidate();
    // END MODIF restore_state
}

void Player::on_main_loop(void *argument)
{
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
    if(suspended && suspend_loops > 0) {
        // if we are suspended we need to allow main loop to cycle a few times then finish off the suspend processing
        if(--suspend_loops == 0) {
            suspend_part2();
            return;
        }
    }
*/
// END MODIF restore_state

    if( !this->booted ) {
        this->booted = true;
        if( this->on_boot_gcode_enable ) {

            // BEGIN MODIF on_boot_without_end_event
            // Don't rise the print begin event as this gcode is run automatically in background on
            // startup
            this->play_command(this->on_boot_gcode, THEKERNEL->serial, false);
            if( this->playing_file ) {
                this->on_boot_gcode_running = true;
            }
            // END MODIF on_boot_without_end_event
        } else {
            //THEKERNEL->serial->printf("On boot gcode disabled! skipping...\n");
        }
    }

    if( this->playing_file ) {
        if(THEKERNEL->is_halted()) {
            abort_command("1", &(StreamOutput::NullStream));
            return;
        }

        char buf[130]; // lines upto 128 characters are allowed, anything longer is discarded
        bool discard = false;

        while(fgets(buf, sizeof(buf), this->current_file_handler) != NULL) {
            int len = strlen(buf);
            if(len == 0) continue; // empty line? should not be possible
            if(buf[len - 1] == '\n' || feof(this->current_file_handler)) {
                if(discard) { // we are discarding a long line
                    discard = false;
                    continue;
                }
                if(len == 1) continue; // empty line

                this->current_stream->printf("%s", buf);
                struct SerialMessage message;
                message.message = buf;
                message.stream = this->current_stream;

                // waits for the queue to have enough room
                THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message);
                played_cnt += len;
                return; // we feed one line per main loop

            } else {
                // discard long line
                this->current_stream->printf("Warning: Discarded long line\n");
                discard = true;
            }
        }

        this->playing_file = false;
        this->filename = "";
        played_cnt = 0;
        file_size = 0;
        fclose(this->current_file_handler);
        current_file_handler = NULL;
        this->current_stream = NULL;

        // BEGIN MODIF restore_state
        // ignore this event when booting (on_boot.gcode)
        // BEGIN MODIF on_boot_without_end_event
        if (!this->on_boot_gcode_running) {
            // not booting. run event
            rise_print_end_event();
        } else {
            // on_boot.gcode finished.
            this->on_boot_gcode_running = false;
        }
        // END MODIF on_boot_without_end_event
        // END MODIF restore_state

        if(this->reply_stream != NULL) {
            // if we were printing from an M command from pronterface we need to send this back
            this->reply_stream->printf("Done printing file\r\n");
            this->reply_stream = NULL;
        }
    }
}

void Player::on_get_public_data(void *argument)
{
    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);

    if(!pdr->starts_with(player_checksum)) return;

// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
    if(pdr->second_element_is(is_playing_checksum) || pdr->second_element_is(is_suspended_checksum)) {
        static bool bool_data;
        bool_data = pdr->second_element_is(is_playing_checksum) ? this->playing_file : this->suspended;
        pdr->set_data_ptr(&bool_data);
        pdr->set_taken();
*/
    if(pdr->second_element_is(is_playing_checksum)) {
        static bool bool_data;
        bool_data = this->playing_file;
        pdr->set_data_ptr(&bool_data);
        pdr->set_taken();
// END MODIF restore_state
    } else if(pdr->second_element_is(get_progress_checksum)) {
        static struct pad_progress p;
        if(file_size > 0 && playing_file) {
            p.elapsed_secs = this->elapsed_secs;
            // BEGIN MODIF time_remaining_calculation
            // p.percent_complete = (this->file_size - (this->file_size - this->played_cnt)) * 100 / this->file_size;
            unsigned long c = this->played_cnt;
            unsigned long s = this->file_size;
            if (s >= ~((unsigned long)0)/101){
                // If the file size is too big, we could have overflow problems. Reduce the resolution.
                c >>= 8;
                s >>= 8;
            }
            p.percent_complete = (c * 100 + s / 2)/ s;
            // END MODIF time_remaining_calculation
            p.filename = this->filename;
            pdr->set_data_ptr(&p);
            pdr->set_taken();
        }
    // BEGIN MODIF time_remaining_calculation
    } else if(pdr->second_element_is(get_detailed_progress_checksum)) {
        static struct detailed_progress p;
        if(file_size > 0 && playing_file) {
            p.effective_secs = this->elapsed_secs - this->ignored_secs;
            p.played_bytes = this->played_cnt;
            p.file_size = this->file_size;
            pdr->set_data_ptr(&p);
            pdr->set_taken();
        }
    // END MODIF time_remaining_calculation
    }
}

void Player::on_set_public_data(void *argument)
{
    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);

    if(!pdr->starts_with(player_checksum)) return;

    if(pdr->second_element_is(abort_play_checksum)) {
        abort_command("", &(StreamOutput::NullStream));
        pdr->set_taken();
    }
}

/**
Suspend a print in progress
1. send pause to upstream host, or pause if printing from sd
1a. loop on_main_loop several times to clear any buffered commmands
2. wait for empty queue
3. save the current position, extruder position, temperatures - any state that would need to be restored
4. retract by specifed amount either on command line or in config
5. turn off heaters.
6. optionally run after_suspend gcode (either in config or on command line)

User may jog or remove and insert filament at this point, extruding or retracting as needed

*/
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
void Player::suspend_command(string parameters, StreamOutput *stream )
{
    if(suspended) {
        stream->printf("Already suspended\n");
        return;
    }

    stream->printf("Suspending print, waiting for queue to empty...\n");

    suspended= true;
    if( this->playing_file ) {
        // pause an sd print
        this->playing_file = false;
        this->was_playing_file= true;
    }else{
        // send pause to upstream host, we send it on all ports as we don't know which it is on
        THEKERNEL->streams->printf("// action:pause\r\n");
        this->was_playing_file= false;
    }

    // we need to allow main loop to cycle a few times to clear any buffered commands in the serial streams etc
    suspend_loops= 10;
}

// this completes the suspend
void Player::suspend_part2()
{
    //  need to use streams here as the original stream may have changed
    THEKERNEL->streams->printf("// Waiting for queue to empty (Host must stop sending)...\n");
    // wait for queue to empty
    THEKERNEL->conveyor->wait_for_empty_queue();

    THEKERNEL->streams->printf("// Saving current state...\n");

    // save current XYZ position
    THEKERNEL->robot->get_axis_position(this->saved_position);

    // save current extruder state
    PublicData::set_value( extruder_checksum, save_state_checksum, nullptr );

    // save state use M120
    THEKERNEL->robot->push_state();

    // TODO retract by optional amount...

    this->saved_temperatures.clear();
    if(!this->leave_heaters_on) {
        // save current temperatures, get a vector of all the controllers data
        std::vector<struct pad_temperature> controllers;
        bool ok = PublicData::get_value(temperature_control_checksum, poll_controls_checksum, &controllers);
        if (ok) {
            // query each heater and save the target temperature if on
            for (auto &c : controllers) {
                // TODO see if in exclude list
                if(c.target_temperature > 0) {
                    this->saved_temperatures[c.id]= c.target_temperature;
                }
            }
        }

        // turn off heaters that were on
        for(auto& h : this->saved_temperatures) {
            float t= 0;
            PublicData::set_value( temperature_control_checksum, h.first, &t );
        }
    }

    // execute optional gcode if defined
    if(!after_suspend_gcode.empty()) {
        struct SerialMessage message;
        message.message = after_suspend_gcode;
        message.stream = &(StreamOutput::NullStream);
        THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message );
    }

    THEKERNEL->streams->printf("// Print Suspended, enter resume to continue printing\n");
}
*/
// END MODIF restore_state

/**
resume the suspended print
1. restore the temperatures and wait for them to get up to temp
2. optionally run before_resume gcode if specified
3. restore the position it was at and E and any other saved state
4. resume sd print or send resume upstream
*/
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
void Player::resume_command(string parameters, StreamOutput *stream )
{
    if(!suspended) {
        stream->printf("Not suspended\n");
        return;
    }

    stream->printf("resuming print...\n");

    // wait for them to reach temp
    if(!this->saved_temperatures.empty()) {
        // set heaters to saved temps
        for(auto& h : this->saved_temperatures) {
            float t= h.second;
            PublicData::set_value( temperature_control_checksum, h.first, &t );
        }
        stream->printf("Waiting for heaters...\n");
        bool wait= true;
        uint32_t tus= us_ticker_read(); // mbed call
        while(wait) {
            wait= false;

            bool timeup= false;
            if((us_ticker_read() - tus) >= 1000000) { // print every 1 second
                timeup= true;
                tus= us_ticker_read(); // mbed call
            }

            for(auto& h : this->saved_temperatures) {
                struct pad_temperature temp;
                if(PublicData::get_value( temperature_control_checksum, current_temperature_checksum, h.first, &temp )) {
                    if(timeup)
                        stream->printf("%s:%3.1f /%3.1f @%d ", temp.designator.c_str(), temp.current_temperature, ((temp.target_temperature == -1) ? 0.0 : temp.target_temperature), temp.pwm);
                    wait= wait || (temp.current_temperature < h.second);
                }
            }
            if(timeup) stream->printf("\n");

            if(wait)
                THEKERNEL->call_event(ON_IDLE, this);

            if(THEKERNEL->is_halted()) {
                // abort temp wait and rest of resume
                THEKERNEL->streams->printf("Resume aborted by kill\n");
                THEKERNEL->robot->pop_state();
                this->saved_temperatures.clear();
                suspended= false;
                return;
            }
        }
    }

    // execute optional gcode if defined
    if(!before_resume_gcode.empty()) {
        stream->printf("Executing before resume gcode...\n");
        struct SerialMessage message;
        message.message = before_resume_gcode;
        message.stream = &(StreamOutput::NullStream);
        THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message );
    }

    // Restore position
    stream->printf("Restoring saved XYZ positions and state...\n");
    THEKERNEL->robot->pop_state();
    bool abs_mode= THEKERNEL->robot->absolute_mode; // what mode we were in
    // force absolute mode for restoring position, then set to the saved relative/absolute mode
    THEKERNEL->robot->absolute_mode= true;
    {
        char buf[128];
        int n = snprintf(buf, sizeof(buf), "G1 X%f Y%f Z%f", saved_position[0], saved_position[1], saved_position[2]);
        string g(buf, n);
        Gcode gcode(g, &(StreamOutput::NullStream));
        THEKERNEL->call_event(ON_GCODE_RECEIVED, &gcode );
    }
    THEKERNEL->robot->absolute_mode= abs_mode;

    // restore extruder state
    PublicData::set_value( extruder_checksum, restore_state_checksum, nullptr );

    stream->printf("Resuming print\n");

    if(this->was_playing_file) {
        this->playing_file = true;
        this->was_playing_file= false;
    }else{
        // Send resume to host
        THEKERNEL->streams->printf("// action:resume\r\n");
    }

    // clean up
    this->saved_temperatures.clear();
    suspended= false;
}
*/
// END MODIF restore_state

// BEGIN MODIF ignore_home_time
void Player::on_autolevel_status_change(void* arg){
    static unsigned long start_time_ignore = uptime();
    autolevel_status_change_t* status = static_cast<autolevel_status_change_t*>(arg);
    if (status->event == al_begin){
        start_time_ignore = uptime();
    } else if (status->event == al_end){
        ignored_secs += uptime() - start_time_ignore;
    }
};
// END MODIF ignore_home_time


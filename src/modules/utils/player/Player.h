/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef PLAYER_H
#define PLAYER_H

#include "Module.h"

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
using std::string;

class StreamOutput;

// BEGIN MODIF restore_state
#include "PrinterStateSnapshot.h"
#include "PrintStatus.h"
// END MODIF restore_state
// BEGIN MODIF time_remaining_calculation
#include "RemainingTimeEstimation.h"
// END MODIF time_remaining_calculation
// BEGIN MODIF printer_mode
#include "PrinterMode.h"
// END MODIF printer_mode

class Player : public Module {
    public:
        Player();
        // BEGIN MODIF restore_state
        void on_config(void* argument);
        // END MODIF restore_state
        // BEGIN MODIF printer_mode
        // Used to speed up some access
        static Player* player;
        friend printer_mode_t get_printer_mode();
        // END MODIF printer_mode

        void on_module_loaded();
        void on_console_line_received( void* argument );
        void on_main_loop( void* argument );
        void on_second_tick(void* argument);
        void on_get_public_data(void* argument);
        void on_set_public_data(void* argument);
        void on_gcode_received(void *argument);

    private:
        // BEGIN MODIF on_boot_without_end_event
        // This method is the real play_command. Parameter was_played_by_user must be true only if
        // the gcode was played by a user, as it will rise the print begin event. By default this
        // parameter is true.
        void play_command( string& parameters, StreamOutput* stream , bool was_played_by_user);
        // END MODIF on_boot_without_end_event
        void play_command( string parameters, StreamOutput* stream );
        void progress_command( string parameters, StreamOutput* stream );
        void abort_command( string parameters, StreamOutput* stream );
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
        void suspend_command( string parameters, StreamOutput* stream );
        void resume_command( string parameters, StreamOutput* stream );
*/
// END MODIF restore_state
        string extract_options(string& args);
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
        void suspend_part2();
*/
// END MODIF restore_state

        string filename;
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
        string after_suspend_gcode;
        string before_resume_gcode;
*/
// END MODIF restore_state
        string on_boot_gcode;
        // BEGIN MODIF on_boot_without_end_event
        bool on_boot_gcode_running;
        // END MODIF on_boot_without_end_event
        // BEGIN MODIF restore_state
        PrinterStateSnapshot pause_state;

//        char** M24_gcode_on_resume;
//        char** M25_gcode_on_pause;
//        char** M26_gcode_on_kill;

        void rise_print_begin_event();
        void rise_print_end_event();
        void rise_print_resume_event();
        void rise_print_pause_event();
        client_printing_state_t get_current_status();
        // END MODIF restore_state


        StreamOutput* current_stream;
        StreamOutput* reply_stream;

        FILE* current_file_handler;
        long file_size;
        unsigned long played_cnt;
        unsigned long elapsed_secs;

// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*

        float saved_position[3];
        std::map<uint16_t, float> saved_temperatures;
*/
// END MODIF restore_state
        struct {
            bool on_boot_gcode_enable:1;
            bool booted:1;
            bool playing_file:1;
// BEGIN MODIF restore_state
// We have a different way of restoring the state after an end of filament, which I feel is better.
/*
            bool suspended:1;
            bool was_playing_file:1;
            bool leave_heaters_on:1;
            uint8_t suspend_loops:4;
*/
// END MODIF restore_state

        };

        // BEGIN MODIF ignore_home_time
        void on_autolevel_status_change(void* arg);
        unsigned long ignored_secs;
        // END MODIF ignore_home_time
        // BEGIN MODIF time_remaining_calculation
        unsigned long calculate_remaining_time();

        RemainingTimeEstimation* estimator;
        // END MODIF time_remaining_calculation
};

#endif // PLAYER_H

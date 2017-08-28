#include "ClientPrint.h"

#include "Kernel.h"
#include "StreamOutputPool.h"
#include "StreamOutput.h"
#include "checksumm.h"
#include "Config.h"
#include "ConfigValue.h"
#include "libs/utils.h"
#include "USBSerial.h"
#include "SerialMessage.h"
#include "PublicData.h"
#include "modules/utils/player/PlayerPublicAccess.h"
#include "GcodeUtils.h"
#include "RobotConsts.h"
#include "lcd_screens.h"

// This constant specifies at which value the counter is reset
#define MAX_COUNTER_RESET 65500
#define LOOK_BACK_COUNT  15


#define clientprint_enable_checksum                        CHECKSUM("clientprint_enable")
#define max_seconds_before_pausing_with_heuristic_checksum CHECKSUM("max_seconds_before_pausing_with_heuristic")
// These G-Code commands will be ran when a M728 is executed. G-Code lines can
// be separated by a \r, \n and ; characters. For that reason, comments are
// NOT allowed, because they could be interpreted as a different line.
// NOTE: The reason why we chose ; is also because G-Code comments are totally
// discouraged!! We don't have that much memory, so we MUSTN'T waste it.
//#define M730_gcode_on_resume_checksum                      CHECKSUM("M730_gcode_on_resume")
//#define M729_gcode_on_pause_checksum                       CHECKSUM("M729_gcode_on_pause")
//#define M728_gcode_on_kill_checksum                        CHECKSUM("M728_gcode_on_kill")
//
//#define DEFAULT_M730_GCODE_ON_RESUME ""
//#define DEFAULT_M729_GCODE_ON_PAUSE "M400;G791;M400;G1 Z30 F" Z_MAX_SPEED_STR ";M400;G790;M400;G1 X0 Y0 F7000;M400"
//#define DEFAULT_M728_GCODE_ON_KILL "G28 X0;M140 S0;M104 S0;M400;M84"
const char* ARRAY_M730_GCODE_ON_RESUME[] = {nullptr};
// NOTE: In order to go to X=MaxX, we can go to X > MaxX as a "trick". In this way, the Robot Module will stop when the X boundary is reached.
const char* ARRAY_M729_GCODE_ON_PAUSE[] = {"M400", "G791", "M400", "G1 Z30 F" Z_MAX_SPEED_STR, "M400", "G790", "M400", "G1 X" X_UPPER_BOUND_STR " Y0 W1 F7000", "M400", nullptr};
const char* ARRAY_M728_GCODE_ON_KILL[] = {"G28 X0", "M140 S0", "M104 S0", "M400", "M84", nullptr};

typedef struct {
    char letter;
    unsigned int gcode_number;
    char param_letter;  // If set to char 0, the parameter will be ignored
    unsigned int param_value;
} gcode_desc_t;
// If these lists are modified, remember to update NUMBER_OF_MATCHING_GCODES_TO_DECLARE_A_BEGINNING and NUMBER_OF_MATCHING_GCODES_TO_DECLARE_AN_ENDING accordingly
const gcode_desc_t beginning_gcodes[] = {{'G', 21, '\0', 0}, {'G', 90, '\0', 0}, {'M', 82, '\0', 0}, {'G', 28, '\0', 0}, {'G', 92, 'E', 0}, {'M', 107, '\0', 0}};
#define NUMBER_OF_MATCHING_GCODES_TO_DECLARE_A_BEGINNING 5
const gcode_desc_t ending_gcodes[] = {{'M', 84, '\0', 0}, {'M', 104, 'S', 0}, {'M', 140, 'S', 0}, {'M', 107, '\0', 0}};
#define NUMBER_OF_MATCHING_GCODES_TO_DECLARE_AN_ENDING 3
const gcode_desc_t ignore_activity_gcodes[] = {{'M', 105, '\0', 0}, {'M', 27, '\0', 0}, {'M', 723, '\0', 0}, {'M', 724, '\0', 0}, {'M', 725, '\0', 0}, {'M', 726, '\0', 0}, {'M', 727, '\0', 0}};


// If you modify this list, remember to update the type client_printing_state_t in ClientPrint.h
const char* CP_STATE[] = {"IDLE", "PRINT", "PAUSE"};


// this variable is used to stop the usb serial in order to kill a print.
extern USBSerial usbserial;

unsigned int last_occurrence_beginning[sizeof(beginning_gcodes)/sizeof(gcode_desc_t)];
unsigned int last_occurrence_ending[sizeof(ending_gcodes)/sizeof(gcode_desc_t)];



ClientPrint::ClientPrint() : pause_state(PrinterStateSnapshot::null_state) {
    this->heuristic_enabled = true;
    this->state = cp_idle;

    // Initialize counters
    this->gcode_counter = LOOK_BACK_COUNT;
    this->last_match = 0;
    this->timestamp_print = 0;

    for (unsigned int i = 0; i < sizeof(beginning_gcodes)/sizeof(gcode_desc_t); i++) {
        last_occurrence_beginning[i] = 0;
    }
    for (unsigned int i = 0; i < sizeof(ending_gcodes)/sizeof(gcode_desc_t); i++) {
        last_occurrence_ending[i] = 0;
    }
    accum_printing_seconds = 0;
    max_seconds_before_pausing_with_heuristic = 0;
    last_activity_timestamp = 0;
//    M728_gcode_on_kill = nullptr;
//    M729_gcode_on_pause = nullptr;
//    M730_gcode_on_resume = nullptr;
}

ClientPrint::~ClientPrint(){
}

void ClientPrint::on_module_loaded(){
    if ( !THEKERNEL->config->value( clientprint_enable_checksum )->by_default(true)->as_bool() ) {
        delete this;
        return;
    }

    this->register_for_event(ON_IDLE);
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_ALERT_TRIGGERED);

    this->on_config(this);
}

void ClientPrint::on_config(void* argument) {
    this->max_seconds_before_pausing_with_heuristic = THEKERNEL->config->value( max_seconds_before_pausing_with_heuristic_checksum )->by_default(10)->as_number();
//    extract_gcode_from_string(this->M728_gcode_on_kill, THEKERNEL->config->value( M728_gcode_on_kill_checksum )->by_default(DEFAULT_M728_GCODE_ON_KILL)->as_string().c_str());
//    extract_gcode_from_string(this->M729_gcode_on_pause, THEKERNEL->config->value( M729_gcode_on_pause_checksum )->by_default(DEFAULT_M729_GCODE_ON_PAUSE)->as_string().c_str());
//    extract_gcode_from_string(this->M730_gcode_on_resume, THEKERNEL->config->value( M730_gcode_on_resume_checksum )->by_default(DEFAULT_M730_GCODE_ON_RESUME)->as_string().c_str());
}

void ClientPrint::on_idle(void* argument){
    if (heuristic_enabled && state == cp_printing) {
        if (uptime() - this->last_activity_timestamp > this->max_seconds_before_pausing_with_heuristic) {
            rise_print_pause_event(true);
            on_guess_gcode_pause();
        }
    }
}

void ClientPrint::rise_print_begin_event(bool estimated) {
    print_status_change_t stat;
//    stat.old_status = state;
    stat.new_status = cp_printing;
    stat.print_source = PS_CLIENT_PRINT;
    stat.event = pe_begin;
    stat.event_was_estimated = estimated;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void ClientPrint::rise_print_end_event(bool estimated) {
    print_status_change_t stat;
//    stat.old_status = state;
    stat.new_status = cp_idle;
    stat.print_source = PS_CLIENT_PRINT;
    stat.event = pe_end;
    stat.event_was_estimated = estimated;
    stat.total_print_time = this->accum_printing_seconds;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void ClientPrint::rise_print_resume_event(bool estimated) {
    print_status_change_t stat;
//    stat.old_status = state;
    stat.new_status = cp_printing;
    stat.print_source = PS_CLIENT_PRINT;
    stat.event = pe_resume;
    stat.event_was_estimated = estimated;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void ClientPrint::rise_print_pause_event(bool estimated) {
    print_status_change_t stat;
//    stat.old_status = state;
    stat.new_status = cp_paused;
    stat.print_source = PS_CLIENT_PRINT;
    stat.event = pe_pause;
    stat.event_was_estimated = estimated;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}

void ClientPrint::on_guess_gcode_beginning() {
    // Restart counter
    accum_printing_seconds = 0;
    // Keep record of the beginning time
    timestamp_print = uptime();

    // Set the current state
    state = cp_printing;
}

void ClientPrint::on_guess_gcode_ending() {
    // End timer
    if (state != cp_paused && state != cp_eof) {
        // Add the remaining time
        accum_printing_seconds += uptime() - timestamp_print;
    }

    // Set the current state
    state = cp_idle;
}

void ClientPrint::on_guess_gcode_pause() {
    // Pause counter
    if (state != cp_paused && state != cp_eof) {
        accum_printing_seconds += uptime() - timestamp_print;
    }

    // Set the current state
    state = cp_paused;
}

void ClientPrint::on_guess_gcode_end_of_filament() {
    // Pause counter
    if (state != cp_paused && state != cp_eof) {
        accum_printing_seconds += uptime() - timestamp_print;
    }

    // Set the current state
    state = cp_eof;
}

void ClientPrint::on_guess_gcode_resume() {
    // Resume counter
    // Keep record of the beginning time
    timestamp_print = uptime();

    // Set the current state
    state = cp_printing;
}
void ClientPrint::increase_counter() {
    // WARN: Never change this variable anywhere different from this place, so that we can catch the equality
    gcode_counter++;
    if (gcode_counter == MAX_COUNTER_RESET) {
        // If we reached a high value, reset the counters.
        gcode_counter = LOOK_BACK_COUNT;

        if (last_match < (unsigned int)(MAX_COUNTER_RESET - LOOK_BACK_COUNT)) {
            last_match = 0;
        } else {
            last_match -= MAX_COUNTER_RESET - LOOK_BACK_COUNT;
        }

        for (unsigned int i = 0; i < sizeof(beginning_gcodes)/sizeof(gcode_desc_t); i++) {
            if (last_occurrence_beginning[i] < (unsigned int)(MAX_COUNTER_RESET - LOOK_BACK_COUNT)) {
                // Too old to be considered. Set 0.
                last_occurrence_beginning[i] = 0;
            } else {
                // Maintain the difference for those that still have chances.
                last_occurrence_beginning[i] -= (MAX_COUNTER_RESET - LOOK_BACK_COUNT);
            }
        }
        for (unsigned int i = 0; i < sizeof(ending_gcodes)/sizeof(gcode_desc_t); i++) {
            if (last_occurrence_ending[i] < (unsigned int)(MAX_COUNTER_RESET - LOOK_BACK_COUNT)) {
                last_occurrence_ending[i] = 0;
            } else {
                last_occurrence_ending[i] -= (MAX_COUNTER_RESET - LOOK_BACK_COUNT);
            }
        }
    }
}

static bool gcode_matches(const gcode_desc_t* gcode_pattern, const Gcode* gcode) {
    // Compare gcode letter and number. If any parameter is specified, validate it too.
    bool match = ((gcode_pattern->letter=='M')?gcode->has_m:(gcode_pattern->letter=='G')?gcode->has_g:false) && 
        ((unsigned int)(gcode_pattern->letter=='M')?gcode->m:(gcode_pattern->letter=='G')?gcode->g:0) == gcode_pattern->gcode_number &&
        (gcode_pattern->param_letter == '\0' || (gcode->has_letter(gcode_pattern->param_letter) && ((unsigned int)gcode->get_value(gcode_pattern->param_letter)) == gcode_pattern->param_value));

    return match;
}

void ClientPrint::update_last_activity_timestamp(Gcode* gcode) {
    // first check if the given gcode must be ignored
    for (unsigned int i = 0; i < sizeof(ignore_activity_gcodes)/sizeof(gcode_desc_t); i++) {
        const gcode_desc_t* test_gcode = &(ignore_activity_gcodes[i]);
        if (gcode_matches(test_gcode, gcode)) {
            return;
        }
    }
    this->last_activity_timestamp = uptime();
    if (heuristic_enabled && state == cp_paused) {
        // We were paused, but now we have received an action. That means printing is being resumed.
        rise_print_resume_event(true);
        on_guess_gcode_resume();
    }
}
void ClientPrint::analyze_gcode_and_guess(Gcode* gcode) {
    if (state == cp_idle) {
        // Don't waste our time checking gcodes that would be ignored in our state.
        // We only care about beginning gcodes if we are not already printing.
        for (unsigned int i = 0; i < sizeof(beginning_gcodes)/sizeof(gcode_desc_t); i++) {
            const gcode_desc_t* test_gcode = &(beginning_gcodes[i]);
            if (gcode_matches(test_gcode, gcode)) {
                // There's a match. This is a candidate to be a beginning of a gcode script!!
                // Mark it with the current timestamp
                last_occurrence_beginning[i] = gcode_counter;
                // Keep track when the last match ocurred to speed up things
                last_match = gcode_counter;
            }
        }
    } else {
        // We only care about ending gcodes if we are already printing.
        for (unsigned int i = 0; i < sizeof(ending_gcodes)/sizeof(gcode_desc_t); i++) {
            const gcode_desc_t* test_gcode = &(ending_gcodes[i]);
            if (gcode_matches(test_gcode, gcode)) {
                // There's a match. This is a candidate to be a ending of a gcode script!!
                // Mark it with the current timestamp
                last_occurrence_ending[i] = gcode_counter;
                // Keep track when the last match ocurred to speed up things
                last_match = gcode_counter;
            }
        }
    }

    if (last_match == gcode_counter) {
        // Something was found this turn. Check if we have enough to declare a beginning or ending.
        // If we find a match, we must clean that state to ensure it's not repeated
        unsigned int oldest_acceptable_stamp = gcode_counter - LOOK_BACK_COUNT;

        int count = 0;
        if (state == cp_idle) {
            // count beginning gcodes
            for (unsigned int i = 0; i < sizeof(beginning_gcodes)/sizeof(gcode_desc_t); i++) {
                if (last_occurrence_beginning[i] >= oldest_acceptable_stamp) {
                    count++;
                }
            }
            // verify if there are enough beginning gcodes
            if (count >= NUMBER_OF_MATCHING_GCODES_TO_DECLARE_A_BEGINNING) {
                // A beginning was detected!!!
                rise_print_begin_event(true);
                on_guess_gcode_beginning();
            }

        } else {
            // count ending gcodes
            for (unsigned int i = 0; i < sizeof(ending_gcodes)/sizeof(gcode_desc_t); i++) {
                if (last_occurrence_ending[i] >= oldest_acceptable_stamp) {
                    count++;
                }
            }
            // verify if there are enough beginning gcodes
            if (count >= NUMBER_OF_MATCHING_GCODES_TO_DECLARE_AN_ENDING) {
                // An ending was detected!!!
                rise_print_end_event(true);
                on_guess_gcode_ending();
            }
        }
    }
}

void ClientPrint::on_gcode_received(void* argument){
    Gcode *gcode = static_cast<Gcode *>(argument);
    if (gcode->has_m && gcode->m == 105) {
        // to ease debugging
        return;
    }

    update_last_activity_timestamp(gcode);

    if (!is_playing_sd_card() && heuristic_enabled) {
        increase_counter();
        // Analyze gcode
        analyze_gcode_and_guess(gcode);
    }

    // Client Printing GCode explicit notifications
    if (gcode->has_m) {
        switch (gcode->m) {
            case 723:   // Notify script beginning
                // We are explicitly told that the script started
                rise_print_begin_event(false);
                on_guess_gcode_beginning();

                // Disable the heuristic
                this->heuristic_enabled = false;
                break;
            case 724:   // Notify script resuming
                if (state == cp_paused) {
                    if ((!gcode->has_letter('F')) || gcode->get_value('F') == 0){    // verify it's not coming from an end of filament event
                        rise_print_resume_event(false);
                    }
                    on_guess_gcode_resume();
                }
                break;
            case 725:   // Notify script pause
                if (state == cp_printing) {
                    rise_print_pause_event(false);
                    on_guess_gcode_pause();
                }
                break;
            case 726:   // Notify script kill
                // We are explicitly told that the script ended
                rise_print_end_event(false);
                on_guess_gcode_ending();

                // Re-eable the heuristic (which had been disabled with M723)
                this->heuristic_enabled = true;
                break;
            case 727:   // Request printing status
                if (state == cp_idle) {
                    gcode->stream->printf("00:00:00|        | |%s\r\n", CP_STATE[state]);
                    return;
                }

                time_t total_printing_seconds;
                long hours;
                int minutes;
                int seconds;
                total_printing_seconds = accum_printing_seconds;
                if (state == cp_printing) {
                    total_printing_seconds += uptime() - timestamp_print;
                }
                convert_to_time_units(total_printing_seconds, &hours, &minutes, &seconds);
                // The format is kept equal to the one in M27 extended: HH:MM:SS|percentage|state
                char time_str[sizeof("00:00:00")];
                format_time(time_str, hours, minutes, seconds);
                gcode->stream->printf("%s|        | |%s\r\n", time_str, CP_STATE[state]);

                break;
            case 728:   // Kill client printing!!! (By stopping the USB communication)
                rise_print_end_event(false);
                on_guess_gcode_ending();
                usbserial.reset();

                // Now run the G-Code that was configured to be executed on M728
//                send_all_gcodes(M728_gcode_on_kill, gcode->stream);
                send_all_gcodes(ARRAY_M728_GCODE_ON_KILL, gcode->stream);

                break;
            case 729:   // Pause by interrupting the USB flow
                if ((!gcode->has_letter('F')) || gcode->get_value('F') == 0){    // verify it's not coming from an end of filament event
                    rise_print_pause_event(false);
                    on_guess_gcode_pause();
                } else {
                    on_guess_gcode_end_of_filament();
                }
                usbserial.set_paused(true);

                if (!this->pause_state.is_valid()) {
                    // In this case we don't have a previous state snapshot. Capture one.
                    pause_state = PrinterStateSnapshot::capture();
//                    send_all_gcodes(M729_gcode_on_pause, gcode->stream);
                    send_all_gcodes(ARRAY_M729_GCODE_ON_PAUSE, gcode->stream);
                }

                break;
            case 730:   // Resume by resuming the USB flow
                if (!gcode->has_letter('P') || gcode->get_value('P') != 0) {
                    rise_print_resume_event(false);
                }
                on_guess_gcode_resume();
                usbserial.set_paused(false);

                if (this->pause_state.is_valid()) {
                    // In this case we are resuming a print, not starting a new one
//                    send_all_gcodes(M730_gcode_on_resume, gcode->stream);
                    send_all_gcodes(ARRAY_M730_GCODE_ON_RESUME, gcode->stream);

                    this->pause_state.restore(gcode->stream);
                    this->pause_state.invalidate();
                }

                break;

            // dual g-codes
            case 824:   // unpause regardless of whether it's printing from an SD card or client
                send_gcode_v(!this->pause_state.is_valid()?"M24 F%d":"M730 F%d", gcode->stream, (!gcode->has_letter('F'))?0:(gcode->get_value('F')!=0.0));
                break;
            case 825:   // pause regardless of whether it's printing from an SD card or client
                send_gcode_v(is_playing_sd_card()?"M25 F%d":"M729 F%d", gcode->stream, (!gcode->has_letter('F'))?0:(gcode->get_value('F')!=0.0));
                break;
            case 826:   // kill job regardless of whether it's printing from an SD card or client
                send_gcode(is_playing_sd_card()?"M26":"M728", gcode->stream);
                break;
            case 827:   // get job status regardless of whether it's printing from an SD card or client
                send_gcode(is_playing_sd_card()?"M27 L1":"M727", gcode->stream);
                break;
        }
    }
}

void ClientPrint::on_alert_triggered(void* argument) {
	// So far all alerts should pause the print
	if (is_printing() || is_playing_sd_card()) {
		send_gcode("M825", &(StreamOutput::NullStream));
		// TODO Probably it's not correct to add this code here from a design
		// point of view (ClientPrint should not depend on LCD code). Move to a
		// better place.
		lcd_screens.pause_screen->draw_screen();
	}
}

bool ClientPrint::is_printing(){
    return state != cp_idle;
}

bool ClientPrint::is_paused(){
    return state == cp_paused;
}


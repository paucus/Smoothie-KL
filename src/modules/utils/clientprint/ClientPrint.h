#ifndef CLIENT_PRINT
#define CLIENT_PRINT

#include "Uptime.h"

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"
#include "libs/StreamOutput.h"
#include "../player/PrinterStateSnapshot.h"
#include "PrintStatus.h"

class ClientPrint : public Module {
public:
    ClientPrint();
    ~ClientPrint();

    void on_module_loaded();
    void on_idle(void* argument);
    void on_config(void* argument);
    void on_gcode_received(void* argument);
    void on_alert_triggered(void* argument);

    bool is_printing();
    bool is_paused();

private:
    void analyze_gcode_and_guess(Gcode* gcode);
    void on_guess_gcode_beginning();
    void on_guess_gcode_ending();
    void on_guess_gcode_pause();
    void on_guess_gcode_end_of_filament();
    void on_guess_gcode_resume();
    void rise_print_begin_event(bool estimated);
    void rise_print_end_event(bool estimated);
    void rise_print_resume_event(bool estimated);
    void rise_print_pause_event(bool estimated);
    void update_last_activity_timestamp(Gcode* gcode);

    void increase_counter();

    bool heuristic_enabled;
    time_t max_seconds_before_pausing_with_heuristic;
//    char** M728_gcode_on_kill;
//    char** M729_gcode_on_pause;
//    char** M730_gcode_on_resume;

    client_printing_state_t state;

    time_t accum_printing_seconds;
    time_t timestamp_print;

    // statistic for heuristic
    unsigned int gcode_counter;
    unsigned int last_match;
    time_t last_activity_timestamp;

    // State to restore
    PrinterStateSnapshot pause_state;
};

#endif // CLIENT_PRINT

#ifndef REPETIER_HOST_NOTIFICATION_H
#define REPETIER_HOST_NOTIFICATION_H

#include "libs/Module.h"
#include "libs/Pin.h"
#include "libs/StreamOutput.h"

#include <bitset>

// This module is in charge of notifying Repetier Host if the filament ran out.
// Enabling report_out_of_filament_event prints a message EndOfFilament when this event occurs.
// Enabling request_pause_on_out_of_filament_event prints a message RequestPause:, which causes
// repetier to pause the printing when this event occurs.
class RepetierHostNotification : public Module{
    public:
        RepetierHostNotification();
        void on_module_loaded();
        void on_config_reload(void* argument);
        void on_gcode_received(void *argument);
        void on_out_of_filament(void* argument);
    private:
        bool report_out_of_filament_event;
        bool request_pause_on_out_of_filament_event;
};

#endif

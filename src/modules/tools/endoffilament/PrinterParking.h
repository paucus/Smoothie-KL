#ifndef PRINTING_PARKING_MODULE_H
#define PRINTING_PARKING_MODULE_H

#include "libs/Module.h"
#include "libs/Pin.h"
#include "libs/StreamOutput.h"

#include <bitset>

#define change_filament_position_checksum               CHECKSUM("change_filament_position")

// This module is responsible of providing the M600 G-Code and to run M600 if the filament runs out.
// M600 pauses a printing job and homes X and Y. Then pauses the current print. If it's being printed
// from the SD card, it performs an SD card pause. If it's being printed from a client, it performs a
// pause interrupting the USB. After that, it retracts the filament and notifies any LCD screen.
// The configuration entry printer_parking_enable_checksum lets you enable or disable the M600 gcode
// completelly.
// If you enable run_M600_on_out_of_filament_event_checksum, the printer will run an M600 command when
// the filament runs out.
class PrinterParking : public Module{
    public:
        PrinterParking();
        void on_module_loaded();
        void on_gcode_received(void* argument);
        void on_gcode_execute(void *argument);
        void on_config_reload(void* argument);
        void on_out_of_filament(void* argument);
        void on_get_public_data(void *);
    private:
        bool run_M600_on_out_of_filament_event;

//        char** M600_gcode_on_eof;
        // BEGIN MODIF change_filament_go_to_center
        // This attribute holds the position where the carriage must go when using the "change
        // filament screen" while not printing.
        float change_filament_position[2];
        // END MODIF change_filament_go_to_center
};

#endif

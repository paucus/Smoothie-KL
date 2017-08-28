#ifndef END_OF_FILAMENT_MODULE_H
#define END_OF_FILAMENT_MODULE_H

#include "libs/Module.h"
#include "libs/Pin.h"

#define end_of_filament_checksum                        CHECKSUM("end_of_filament")
#define is_out_of_filament_checksum                     CHECKSUM("is_out_of_filament")

// This module is responsible for detecting when the filament ran out by sensing an endstop pin.
class EndOfFilament : public Module{
    public:
        EndOfFilament();
        void on_module_loaded();
        void on_gcode_received(void* argument);
        void on_gcode_execute(void* argument);
        void on_config_reload(void* argument);
        void on_get_public_data(void *argument);
        void on_main_loop(void *argument);
    private:
        Pin pin;
        bool out_of_filament;
        bool out_of_filament_reported;
        bool end_of_filament_enabled;

        // this variable lets us move the end of filament logic to on_idle event, which allow us to
        // work with dynamic memory.
        bool is_now_extrusion_gcode;
};

#endif

/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KERNEL_H
#define KERNEL_H

#define THEKERNEL Kernel::instance

#include "Module.h"
#include <array>
#include <vector>
#include <queue>
#include <string>
// BEGIN MODIF display
#include "SerialMessage.h"
// END MODIF display

//Module manager
class Config;
class Module;
class Conveyor;
class SlowTicker;
class Pauser;
class SerialConsole;
class StreamOutputPool;
class GcodeDispatch;
class Robot;
class Stepper;
class Planner;
class StepTicker;
class Adc;
class PublicData;
class TemperatureControlPool;
class Gcode;
class StreamOutput;

class Kernel {
    public:
        Kernel();
        static Kernel* instance; // the Singleton instance of Kernel usable anywhere
        // BEGIN MODIF second_config_file
        const char* config_override_filename(){ return "/" CONFIG_SD_MOUNT_DIR "/config-override"; }
        const char* config_priv_filename(){ return "/" CONFIG_SD_MOUNT_DIR "/config-priv"; }
        // END MODIF second_config_file

        void add_module(Module* module);
        void register_for_event(_EVENT_ENUM id_event, Module *module);
        void call_event(_EVENT_ENUM id_event, void * argument= nullptr);

        bool kernel_has_event(_EVENT_ENUM id_event, Module *mod);
        void unregister_for_event(_EVENT_ENUM id_event, Module *module);

        bool is_using_leds() const { return use_leds; }
        bool is_halted() const { return halted; }

        void process_display_gcode_queue();
        void append_gcode_to_queue(Gcode* gcode);
        void append_gcode_to_queue(const char* msg, StreamOutput* output);
        void append_gcode_v_to_queue(const char* msg, StreamOutput* output, ...);

        // These modules are available to all other modules
        SerialConsole*    serial;
        StreamOutputPool* streams;

        Robot*            robot;
        Stepper*          stepper;
        Planner*          planner;
        Config*           config;
        Conveyor*         conveyor;
        Pauser*           pauser;

        int debug;
        SlowTicker*       slow_ticker;
        StepTicker*       step_ticker;
        Adc*              adc;
        std::string       current_path;
        uint32_t          base_stepping_frequency;
        uint32_t          acceleration_ticks_per_second;

    private:
        // When a module asks to be called for a specific event ( a hook ), this is where that request is remembered
        std::array<std::vector<Module*>, NUMBER_OF_DEFINED_EVENTS> hooks;
        // BEGIN MODIF display
        std::queue<SerialMessage*> display_gcodes;
        // END MODIF display
        struct {
            bool use_leds:1;
            bool halted:1;
        };

};

#endif

#ifndef SERIAL_NUMBER_H
#define SERIAL_NUMBER_H

#include <string>
using std::string;
#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"

#include "libs/StreamOutput.h"

#define SERIAL_NUMBER_MAX_LENGTH 20

class SerialNumber : public Module {
    public:
        SerialNumber();

        static string serial_number;
        virtual void on_module_loaded();
        virtual void on_config_reload(void *argument);
        virtual void on_gcode_received(void *argument);
};

#endif

#include "SerialNumber.h"

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"
#include "libs/utils.h"
#include "Config.h"
#include "checksumm.h"
#include "ConfigValue.h"


#define serial_number_checksum          CHECKSUM("serial_number")

string SerialNumber::serial_number = "";

SerialNumber::SerialNumber() {
}

void SerialNumber::on_module_loaded()
{
    this->register_for_event(ON_GCODE_RECEIVED);
    
    on_config_reload(this);
}


void SerialNumber::on_config_reload(void *argument) {
    SerialNumber::serial_number = THEKERNEL->config->value( serial_number_checksum )->by_default( "" )->as_string();
}

// When a command is received, if it is a Gcode, dispatch it as an object via an event
void SerialNumber::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);

    if (gcode->has_m) {
        if (gcode->m == 550) {
            string param = get_arguments(gcode->get_command());
            if (param.size() > SERIAL_NUMBER_MAX_LENGTH) param = param.substr(0, SERIAL_NUMBER_MAX_LENGTH);
            SerialNumber::serial_number = param;
        } else if (gcode->m == 513 || gcode->m == 510) { // M513 Print the private settings && M510 Store private settings
            gcode->stream->printf(";Serial Number:\nM550 %s\n", SerialNumber::serial_number.c_str());
        }
    }
}

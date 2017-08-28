#include "libs/Module.h"
#include "libs/Kernel.h"
#include "modules/robot/Conveyor.h"
#include "RepetierHostNotification.h"
#include "Config.h"
#include "ConfigValue.h"
#include "Planner.h"
#include "Gcode.h"
#include "checksumm.h"
#include "libs/StreamOutput.h"
#include "libs/StreamOutputPool.h"

#define report_out_of_filament_event_checksum           CHECKSUM("report_out_of_filament_event")
#define request_pause_on_out_of_filament_event_checksum CHECKSUM("request_pause_on_out_of_filament_event")


RepetierHostNotification::RepetierHostNotification()
{
    this->report_out_of_filament_event = true;
    this->request_pause_on_out_of_filament_event = true;
}

void RepetierHostNotification::on_module_loaded()
{
    //register_for_event(ON_CONFIG_RELOAD);
    register_for_event(ON_OUT_OF_FILAMENT);
    register_for_event(ON_GCODE_RECEIVED);

    // Settings
    this->on_config_reload(this);
}

// Get config
void RepetierHostNotification::on_config_reload(void *argument)
{
    this->report_out_of_filament_event = THEKERNEL->config->value(report_out_of_filament_event_checksum)->by_default(false)->as_bool();
    this->request_pause_on_out_of_filament_event = THEKERNEL->config->value(request_pause_on_out_of_filament_event_checksum)->by_default(false)->as_bool();
}

void RepetierHostNotification::on_gcode_received(void *argument)
{
    Gcode* gcode = static_cast<Gcode*>(argument);
    if (gcode->has_m) {
        switch (gcode->m) {
            case 45: //M45 - Send "RequestPause:" message to client host when the printer runs out of material. This will cause a pause in RepetierHost.
                this->request_pause_on_out_of_filament_event = true;
            break;
            case 46: //M46 - Disable sending "RequestPause:" when the printer runs out of material.
                this->request_pause_on_out_of_filament_event = false;
            break;
            case 47: //M47 - Report "Event:EndOfFilament" to client host when the printer runs out of material. This can be handled by the client host if it supports this.
                this->report_out_of_filament_event = true;
            break;
            case 48: //M48 - Disable reporting "Event:EndOfFilament" to client host when the printer runs out of material.
                this->report_out_of_filament_event = false;
            break;
        }
    }
}

// Start homing sequences by response to GCode commands
void RepetierHostNotification::on_out_of_filament(void *argument)
{
    if (this->report_out_of_filament_event)
    {
        THEKERNEL->streams->printf("Event:EndOfFilament\r\n");
    }
    if (this->request_pause_on_out_of_filament_event)
    {
        THEKERNEL->streams->printf("RequestPause:\r\n");
    }
}

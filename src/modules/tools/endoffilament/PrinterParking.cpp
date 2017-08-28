#include "libs/Module.h"
#include "libs/Kernel.h"
#include "modules/communication/utils/Gcode.h"
#include "modules/robot/Conveyor.h"
#include "PrinterParking.h"
#include "Config.h"
#include "ConfigValue.h"
#include "Planner.h"
#include "checksumm.h"
#include "modules/utils/player/PlayerPublicAccess.h"
#include "libs/SerialMessage.h"
#include "PublicData.h"
#include "Gcode.h"
#include "modules/communication/utils/GcodeUtils.h"
#include "PrintStatus.h"
// BEGIN MODIF change_filament_go_to_center
#include "Robot.h"
#include "nuts_bolts.h"
#include "PublicDataRequest.h"
#define change_filament_position_x_checksum             CHECKSUM("change_filament_position_x")
#define change_filament_position_y_checksum             CHECKSUM("change_filament_position_y")
// END MODIF change_filament_go_to_center


#define printer_parking_enable_checksum                 CHECKSUM("printer_parking_enable")
#define run_M600_on_out_of_filament_event_checksum      CHECKSUM("run_M600_on_out_of_filament_event")
#define M600_gcode_on_eof_checksum                      CHECKSUM("M600_gcode_on_eof")

// Enqueue a pause and store state, retract some mms, then go Home X and Y, lift Z, extrude a bit to
// ensure it won't get stuck in the end of filament endstop, then retract a lot.
//#define DEFAULT_M600_GCODE_ON_EOF "M825 F1;M400;M744;M702 P0;M400;G791;M400;G1 E15 F320;M400;G1 E-80 F700;M400;M702 P1;M743;G790;M400"
const char* ARRAY_M600_GCODE_ON_EOF[] = {"M825 F1", "M400", "M744", "M702 P0", "M400", "G791", "M400", "G1 E15 F320", "M400", "G1 E-80 F700", "M400", "M702 P1", "M743", "G790", "M400", nullptr};

PrinterParking::PrinterParking()
{
    this->run_M600_on_out_of_filament_event = true;
}

void PrinterParking::on_module_loaded()
{
    register_for_event(ON_GCODE_RECEIVED);
    register_for_event(ON_GCODE_EXECUTE);
    register_for_event(ON_OUT_OF_FILAMENT);
    register_for_event(ON_GET_PUBLIC_DATA);

    // Settings
    this->on_config_reload(this);
    // BEGIN MODIF change_filament_go_to_center
    // By default go to the center of the bed
    change_filament_position[X_AXIS] = THEKERNEL->config->value(change_filament_position_x_checksum)->by_default((THEKERNEL->robot->max_coord_value[X_AXIS] + THEKERNEL->robot->min_coord_value[X_AXIS]) / 2)->as_number();
	change_filament_position[Y_AXIS] = THEKERNEL->config->value(change_filament_position_y_checksum)->by_default((THEKERNEL->robot->max_coord_value[Y_AXIS] + THEKERNEL->robot->min_coord_value[Y_AXIS]) / 2)->as_number();
    // END MODIF change_filament_go_to_center
}

// Get config
void PrinterParking::on_config_reload(void *argument)
{
    // Do not do anything if not enabled
    if ( THEKERNEL->config->value( printer_parking_enable_checksum )->by_default(false)->as_bool() == false ) {
        return;
    }

    this->run_M600_on_out_of_filament_event = THEKERNEL->config->value(run_M600_on_out_of_filament_event_checksum)->by_default(false)->as_bool();

//    extract_gcode_from_string(this->M600_gcode_on_eof, THEKERNEL->config->value( M600_gcode_on_eof_checksum )->by_default(DEFAULT_M600_GCODE_ON_EOF)->as_string().c_str());
}

static void rise_end_of_filament_event() {
    print_status_change_t stat;
//    stat.old_status = get_current_status();	// temporarily disabled this
    stat.new_status = cp_eof;
    stat.print_source = PS_UNKNOWN;	// we could be either printing with SD card or from USB. It's independent.
    stat.event = pe_eof;
    stat.event_was_estimated = false;
    THEKERNEL->call_event(ON_PRINT_STATUS_CHANGE, &stat);
}
void PrinterParking::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);
    if (gcode->has_m) {
        switch (gcode->m) {
            case 43: //M43 - Stop if out of material and leave heated bed on (end of filament detection controlled by printer)
                this->run_M600_on_out_of_filament_event = true;
            break;
            case 44: //M44 - Disable end of filament detection controlled by printer
                this->run_M600_on_out_of_filament_event = false;
            break;
            case 600:
                THEKERNEL->conveyor->append_gcode(gcode);
                // ensure that no subsequent gcodes get executed along with it
                THEKERNEL->conveyor->queue_head_block();
            break;
        }
    }
}

void PrinterParking::on_gcode_execute(void *argument)
{
	Gcode *gcode = static_cast<Gcode *>(argument);
	if (gcode->has_m && gcode->m == 600){
		// pretend that an out of filament event ocurred
		THEKERNEL->call_event(ON_OUT_OF_FILAMENT);
	}
}

void PrinterParking::on_out_of_filament(void *argument)
{
    if (this->run_M600_on_out_of_filament_event)
    {
        // queue a printer parking command
        THEKERNEL->conveyor->wait_for_empty_queue();

//        send_all_gcodes(M600_gcode_on_eof, &(StreamOutput::NullStream));
        send_all_gcodes(ARRAY_M600_GCODE_ON_EOF, &(StreamOutput::NullStream));

        rise_end_of_filament_event();
    }
}

void PrinterParking::on_get_public_data(void * args) {
	PublicDataRequest* pdr = static_cast<PublicDataRequest*>(args);
	if (pdr->starts_with(change_filament_position_checksum)) {
		pdr->set_data_ptr(change_filament_position);
		pdr->set_taken();
	}
}

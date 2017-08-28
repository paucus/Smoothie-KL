#include "libs/Module.h"
#include "libs/Kernel.h"
#include "modules/communication/utils/Gcode.h"
#include "modules/robot/Conveyor.h"
#include "EndOfFilament.h"
#include "libs/Pin.h"
#include "PublicData.h"
#include "Config.h"
#include "ConfigValue.h"
#include "Planner.h"
#include "checksumm.h"
#include "libs/StreamOutput.h"
#include "libs/SerialMessage.h"
#include "PublicDataRequest.h"
#include "ExtruderPublicAccess.h"
#include "Gcode.h"

#define end_of_filament_enable_checksum                 CHECKSUM("end_of_filament_enable")
#define end_of_filament_pin_checksum                    CHECKSUM("end_of_filament_pin")


EndOfFilament::EndOfFilament()
{
    this->out_of_filament = false;
    this->out_of_filament_reported = false;
    this->end_of_filament_enabled = true;
    this->is_now_extrusion_gcode = false;
}

void EndOfFilament::on_module_loaded()
{
    // Do not do anything if not enabled
    if ( THEKERNEL->config->value( end_of_filament_enable_checksum )->by_default(false)->as_bool() == false ) {
        return;
    }

    //register_for_event(ON_CONFIG_RELOAD);
    register_for_event(ON_GCODE_RECEIVED);
    register_for_event(ON_GCODE_EXECUTE);
    register_for_event(ON_GET_PUBLIC_DATA);
    register_for_event(ON_MAIN_LOOP);

    // Settings
    this->on_config_reload(this);
}

// Get config
void EndOfFilament::on_config_reload(void *argument)
{
    this->pin.from_string( THEKERNEL->config->value(end_of_filament_pin_checksum)->by_default("nc" )->as_string())->as_input();
}

static bool is_absolute_mode() {
    bool returned_e_abs;
    if (!PublicData::get_value( extruder_checksum, absolute_mode_checksum, &returned_e_abs )) {
        return true;
    } else {
        return returned_e_abs;
    }
}
static float get_current_e_position() {
    float returned_e;
    if (!PublicData::get_value( extruder_checksum, target_position_checksum, &returned_e )) {
        return -1;
    } else {
        return returned_e;
    }
}

static bool is_extrusion_gcode(Gcode* gcode){
    return gcode->has_g && (gcode->g == 1 || gcode->g == 0) && gcode->has_letter('E') &&
            (is_absolute_mode()?(gcode->get_value('E') > get_current_e_position()):(gcode->get_value('E') > 0));
}

// Start homing sequences by response to GCode commands
void EndOfFilament::on_gcode_received(void *argument)
{
    Gcode* gcode = static_cast<Gcode*>(argument);
    if (gcode->has_m && (gcode->m == 743 || gcode->m == 744)) {    // Enable/Disable End of filament detection
        THEKERNEL->conveyor->append_gcode(gcode);
        // ensure that no subsequent gcodes get executed along with it
        THEKERNEL->conveyor->queue_head_block();
    } else if (gcode->has_m && gcode->m == 745) {    // M745: Filament detected?
        if (gcode->has_letter('F')){
            this->out_of_filament = !(gcode->get_value('F') != 0.0);
        }
        if (gcode->has_letter('R')){
            this->out_of_filament_reported = (gcode->get_value('R') != 0.0);
        }
        gcode->stream->printf("filament_present(F): %d reported(R): %d\r\n", out_of_filament?0:1, out_of_filament_reported?1:0);
    }
}


void EndOfFilament::on_gcode_execute(void *argument)
{
    Gcode* gcode = static_cast<Gcode*>(argument);
    if (gcode->has_m && gcode->m == 743) {    // Enable End of filament detection
        this->end_of_filament_enabled = true;
    } else if (gcode->has_m && gcode->m == 744) {    // Disable End of filament detection
        this->end_of_filament_enabled = false;
        // just in case, mark any possible out of filament event as reported
        this->out_of_filament_reported = true;
    }


    bool filament_pin_status = this->pin.get();
    
    bool was_out_of_filament_before = out_of_filament;
    
    out_of_filament = !filament_pin_status;
    
    if ( out_of_filament && !was_out_of_filament_before ) {
        this->out_of_filament_reported = false;
    }

    if (!end_of_filament_enabled) {
        is_now_extrusion_gcode = false;
        return;
    }

    is_now_extrusion_gcode = is_extrusion_gcode(gcode);
}

void EndOfFilament::on_main_loop(void *argument)
{
	if (!end_of_filament_enabled) {
		return;
	}
	// detect extrusion and verify if we are out of filament!!
	if (out_of_filament && !out_of_filament_reported && is_now_extrusion_gcode ) {
		out_of_filament_reported = true;
		THEKERNEL->call_event(ON_OUT_OF_FILAMENT);
		is_now_extrusion_gcode = false;
	}
}

void EndOfFilament::on_get_public_data(void *argument)
{
    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);

    if(!pdr->starts_with(end_of_filament_checksum)) return;

    if(pdr->second_element_is(is_out_of_filament_checksum)) {
        (*static_cast<bool*>(pdr->get_data_ptr())) = out_of_filament;
        pdr->set_taken();
    }
}

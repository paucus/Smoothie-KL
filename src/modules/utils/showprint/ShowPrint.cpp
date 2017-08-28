/*
 * ShowPrint.cpp
 *
 *  Created on: Feb 24, 2015
 *      Author: eai
 */

#include "ShowPrint.h"
#include "Kernel.h"
#include "checksumm.h"
#include "PrintStatus.h"
#include "StreamOutput.h"
#include "StreamOutputPool.h"
#include "Config.h"
#include "ConfigValue.h"
#include "Robot.h"
#include "nuts_bolts.h"
#include "GcodeUtils.h"
#include "PublicData.h"
#include "EndstopsPublicAccess.h"
#include "RobotConsts.h"

#define show_print_checksum             CHECKSUM("show_print")
#define enable_checksum                 CHECKSUM("enable")
#define go_at_least_to_z_checksum       CHECKSUM("go_at_least_to_z")
#define lift_z_checksum                 CHECKSUM("lift_z")
#define z_speed_checksum                CHECKSUM("z_speed")

ShowPrint::ShowPrint() {
}

ShowPrint::~ShowPrint() {
}

void ShowPrint::on_module_loaded() {
    if (!THEKERNEL->config->value( show_print_checksum, enable_checksum )->by_default(true)->as_bool() ) {
        delete this;
        return;
    }
    this->go_at_least_to_z = THEKERNEL->config->value( show_print_checksum, go_at_least_to_z_checksum )->by_default( 200.0f )->as_number();
    this->lift_z = THEKERNEL->config->value( show_print_checksum, lift_z_checksum )->by_default( 50.0f )->as_number();
    this->z_speed = THEKERNEL->config->value( show_print_checksum, z_speed_checksum )->by_default( Z_MAX_SPEED )->as_number();
    this->register_for_event(ON_PRINT_STATUS_CHANGE);
    this->register_for_event(ON_GCODE_RECEIVED);
}

void ShowPrint::on_print_status_change(void* arg) {
    print_status_change_t* status = static_cast<print_status_change_t*>(arg);
    if (status->event == pe_end && enable_show_print) {
        send_gcode("M741", THEKERNEL->streams);
    }
}

static float get_current_z() {
    float pos[3];
    THEKERNEL->robot->get_axis_position(pos);
    float go_to_z = pos[Z_AXIS];
    return go_to_z;
}

static bool is_axis_position_known_z() {
	bool* axis_position_known;
	if (PublicData::get_value(endstops_checksum, axis_position_known_checksum, (void**)&axis_position_known)) {
		return axis_position_known[Z_AXIS];
	} else {
		return false;
	}
}

void ShowPrint::on_gcode_received(void* arg) {
    Gcode* gcode = static_cast<Gcode*>(arg);
    if (gcode->has_m) {
        if (gcode->m == 741) {
            if (!is_axis_position_known_z()) {
                // If we don't know our current Z position, DON'T MOVE! We could break the printer
                return;
            }

            float z_speed_par =
                    (gcode->has_letter('F')) ?
                            gcode->get_value('F') : this->z_speed;
            // Check configuration of expected Z position and override the value with any given z. This value MUST be positive.
            float go_at_least_to_z_par = max(0.0f,
                    (gcode->has_letter('Z')) ?
                            gcode->get_value('Z') : this->go_at_least_to_z);
            // Check configuration and override the value with any given delta z. This value MUST be positive or we will break the printed object.
            float z_addition = max(0.0f,
                    (gcode->has_letter('K')) ?
                            gcode->get_value('K') : this->lift_z);
            float z = get_current_z() + z_addition;

            // Ensure that we move up to the expected position at least
            z = max(z, go_at_least_to_z_par);
            // Limit z value to the printable volume
            float max_z = THEKERNEL->robot->max_coord_value[Z_AXIS];
            z = min(z, max_z);
            send_gcode_v("G1 Z%0.4f F%0.4f", gcode->stream, z, z_speed_par);
        } else if (gcode->m == 742) {
            if (gcode->has_letter('S')) {
                this->enable_show_print = (gcode->get_value('S') != 0);
            }
            gcode->stream->printf("show print %s (enable/disable with S)\r\n",
                    this->enable_show_print ? "enabled" : "disabled");
        }
    }
}


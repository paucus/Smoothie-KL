#include "PrinterStateSnapshot.h"

#include "libs/Kernel.h"
#include "Conveyor.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"
#include "Gcode.h"
#include "checksumm.h"
#include "PublicData.h"
#include "PublicDataRequest.h"
#include "modules/tools/extruder/ExtruderPublicAccess.h"
#include "Robot.h"
#include "libs/SerialMessage.h"
#include "modules/communication/utils/GcodeUtils.h"
#include "RobotConsts.h"

PrinterStateSnapshot PrinterStateSnapshot::null_state;

PrinterStateSnapshot::PrinterStateSnapshot() {
    this->valid = false;
}
PrinterStateSnapshot::PrinterStateSnapshot(float e, float x, float y, float z, float f, float s, bool r_abs, bool e_abs) {
    this->e = e;
    this->x = x;
    this->y = y;
    this->z = z;
    this->f = f;
    this->s = s;
    this->r_abs = r_abs;
    this->e_abs = e_abs;
    this->valid = true;
}
PrinterStateSnapshot::PrinterStateSnapshot(PrinterStateSnapshot& snap) {
    this->e = snap.e;
    this->x = snap.x;
    this->y = snap.y;
    this->z = snap.z;
    this->f = snap.f;
    this->s = snap.s;
    this->r_abs = snap.r_abs;
    this->e_abs = snap.e_abs;
    this->valid = snap.valid;
}
PrinterStateSnapshot::PrinterStateSnapshot(const PrinterStateSnapshot& snap) {
    this->e = snap.e;
    this->x = snap.x;
    this->y = snap.y;
    this->z = snap.z;
    this->f = snap.f;
    this->s = snap.s;
    this->r_abs = snap.r_abs;
    this->e_abs = snap.e_abs;
    this->valid = snap.valid;
}
bool PrinterStateSnapshot::is_valid() {
    return this->valid;
}
void PrinterStateSnapshot::restore(StreamOutput* stream) {
	this->restore(stream, true);
}
void PrinterStateSnapshot::restore(StreamOutput* stream, bool restore_position) {
    // If the snapshot is not valid, don't do anything
    if (!this->valid)
        return;

    // Wait before restoring the state until all movements are completed
    THEKERNEL->conveyor->wait_for_empty_queue();
    send_gcode("G790", stream);  // set absolute mode temporarily
    send_gcode_v("G92 E%.7f", stream, this->e);      // restore e

    // restore position
    float restore_state_speed = XY_MAX_SPEED; // TODO make it configurable
    float restore_state_speed_z = Z_MAX_SPEED; // TODO make it configurable

    float x, y, z;
    if (restore_position) {
        x = this->x;
        y = this->y;
        z = this->z;
    } else {
        // if we are not requested to restore position, just move to current position
        float robot_position[3];
        THEKERNEL->robot->get_axis_position(robot_position);
        x = robot_position[X_AXIS];
        y = robot_position[Y_AXIS];
        z = robot_position[Z_AXIS];
    }
    send_gcode_v("G1 X%.3f Y%.3f F%.1f", stream, x, y, restore_state_speed);
    send_gcode_v("G1 Z%.3f F%.1f", stream, z, restore_state_speed_z);

    if (this->r_abs) {
        send_gcode("G790", stream);
    } else {
        send_gcode("G791", stream);
    }
    if (this->e_abs) {
        send_gcode("M82", stream);
    } else {
        send_gcode("M83", stream);
    }

    // Go to the same position setting speed
    send_gcode_v("G0 F%.3f", stream, this->s);      // restore s (seek rate)
    send_gcode_v("G0 X%.3f Y%.3f Z%.3f F%.1f", stream, x, y, z, this->s);
    send_gcode_v("G1 F%.3f", stream, this->f);      // restore f
    send_gcode_v("G1 X%.3f Y%.3f Z%.3f F%.1f", stream, x, y, z, this->f);

    // Wait before continuing
    THEKERNEL->conveyor->wait_for_empty_queue();
}

PrinterStateSnapshot PrinterStateSnapshot::capture() {
    // Wait until all movements are completed before capturing the state, or we will capture a
    // state that is not the last one before pausing.
    THEKERNEL->conveyor->wait_for_empty_queue();

    bool returned_e_abs;
    float returned_e;
    float returned_f;
    float returned_s;
    if (!PublicData::get_value( extruder_checksum, target_position_checksum, &returned_e ))
        return null_state;
    if (!PublicData::get_value( extruder_checksum, absolute_mode_checksum, &returned_e_abs ))
        return null_state;
    returned_f = THEKERNEL->robot->get_feed_rate();
    returned_s = THEKERNEL->robot->get_seek_rate();

    float robot_position[3];
    THEKERNEL->robot->get_axis_position(robot_position);
    return PrinterStateSnapshot(returned_e, robot_position[X_AXIS], robot_position[Y_AXIS], robot_position[Z_AXIS], returned_f, returned_s, THEKERNEL->robot->absolute_mode, returned_e_abs);
}

void PrinterStateSnapshot::invalidate() {
    this->valid = false;
}

/*
 * MiscModule.cpp
 *
 *  Created on: Mar 2, 2016
 *      Author: eai
 */

#include "MiscModule.h"

#include "Kernel.h"
#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
#include "StreamOutput.h"
#include "PrintStatus.h"
#include "GcodeUtils.h"

#define misc_module_enable_checksum            CHECKSUM("misc_module_enable")

MiscModule::MiscModule() {
}

MiscModule::~MiscModule() {
}

void MiscModule::on_module_loaded() {
	if (!THEKERNEL->config->value(misc_module_enable_checksum)->by_default(true)->as_bool()) {
		delete this;
		return;
	}
	this->register_for_event(ON_PRINT_STATUS_CHANGE);
}
void MiscModule::on_print_status_change(void * args) {
	// Turn the FAN off when printing finishes
	print_status_change_t* ev = static_cast<print_status_change_t*>(args);
	if (ev->event == pe_end) {
		send_gcode("M107", &(StreamOutput::NullStream));
	}
}


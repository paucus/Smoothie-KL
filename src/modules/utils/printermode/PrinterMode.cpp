/*
 * PrinterMode.cpp
 *
 *  Created on: Dec 2, 2015
 *      Author: eai
 */

#include "PrinterMode.h"
#include "utils.h"
#include "USBSerial.h"
#include "Player.h"
#include "PlayerPublicAccess.h"
#include "telnetd.h"
#include "utils.h"
#include "Config.h"
#include "ConfigValue.h"
#include "Kernel.h"
#include "checksumm.h"
#include "Gcode.h"
#include "connection_event.h"

#define printer_mode_checksum    CHECKSUM("printer_mode")
#define enabled_checksum         CHECKSUM("enabled")

// If you modify this list, update printer_mode enum in PrinterMode.h
const char* PRINTER_MODE_NAMES[] = {"NONE", "TELNET", "USB", "SD", "DISABLED"};

// These commands will go through even if the printer mode doesn't allow commands from that source.
// They are mostly for control purposes.
// WARN: The list MUST be NUMERICALLY ORDERED, so as to take advantage of this to make the comparison faster.
// G codes MUST be before M codes. For example G10, G21, G101 is ok. G10, G101, G21 is wrong. G10,
// M3 is ok. M3, G10 is wrong.
static const char* WHITELISTED_COMMANDS[] = {"M24", "M25", "M26", "M27", "M104", "M105", "M140", "M805", NULL};

extern USBSerial usbserial;
printer_mode_t get_printer_mode() {
	if (!PrinterMode::mode_restrictions_enabled) {
		return pm_disabled;
	}
	// NOTE: This is not the real right way to do this. However, as this code
	// will be called very frequently, it's better to skip some barriers.
	// Calling "get public data" would be the right way, but it's slower.
	if (Player::player->playing_file) {
		return pm_sd;
	} else if (usbserial.attached) {
		return pm_usb;
	} else if (Telnetd::num_conns > 0) {
		return pm_telnet;
	} else {
		return pm_none;
	}
}
bool command_is_in_whitelist(const char* line) {
	int i = 0;
	while(WHITELISTED_COMMANDS[i]){
		const char* trimmed_line = trim_left_cstr(line);
		int res = is_command(trimmed_line, WHITELISTED_COMMANDS[i]);
		if (res == 0){
			return true;
		} else if (res == -1) {
			// We are sure that from here on the g-code won't match.
			return false;
		}
		i++;
	}
	return false;
}

bool PrinterMode::mode_restrictions_enabled = false;

PrinterMode::PrinterMode() {
}
void PrinterMode::on_module_loaded() {
	PrinterMode::mode_restrictions_enabled = THEKERNEL->config->value(printer_mode_checksum, enabled_checksum)->by_default(true)->as_bool();

	if (!PrinterMode::mode_restrictions_enabled) {
		delete this;
		return;
	}
	this->register_for_event(ON_GCODE_RECEIVED);
}

void PrinterMode::on_gcode_received(void * args) {
	Gcode* gcode = static_cast<Gcode*>(args);
	if (gcode->has_m && gcode->m == 805) { // Enable/Disable printer mode restrictions
		if (gcode->has_letter('E')) {
			int e = gcode->get_int('E');
			PrinterMode::mode_restrictions_enabled = e;
			connection_event_t evt;
			evt.src = cs_restrictions;
			evt.rest_action = e?ret_enabled_restrictions:ret_disabled_restrictions;
			THEKERNEL->call_event(ON_CONNECTION_CHANGE, &evt);
		}
		gcode->stream->printf("Current mode: %s (Enable/Disable with E1/0)\r\n", PRINTER_MODE_NAMES[get_printer_mode()]);
	}
}

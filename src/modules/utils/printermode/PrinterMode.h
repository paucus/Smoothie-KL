/*
 * PrinterMode.h
 *
 *  Created on: Dec 2, 2015
 *      Author: eai
 */

#ifndef PRINTERMODE_H_
#define PRINTERMODE_H_

// If you modify this list, update PRINTER_MODE_NAMES const in PrinterMode.cpp
typedef enum printer_mode { pm_none, pm_telnet, pm_usb, pm_sd, pm_disabled } printer_mode_t;

printer_mode_t get_printer_mode();
bool command_is_in_whitelist(const char* line);

#include "Module.h"
class PrinterMode : public Module {
public:
	PrinterMode();
	void on_module_loaded();
	void on_gcode_received(void *);
	static bool mode_restrictions_enabled;
};


inline bool usb_commands_allowed() {
	if (!PrinterMode::mode_restrictions_enabled) {
		return true;
	}
	return get_printer_mode() == pm_usb;
}
inline bool telnet_commands_allowed() {
	if (!PrinterMode::mode_restrictions_enabled) {
		return true;
	}
	return get_printer_mode() == pm_telnet;
}
inline bool sd_print_allowed() {
	// So far I haven't found a case where SD must be disabled.
	return true;
}
inline bool pause_and_kill_print_allowed() {
	if (!PrinterMode::mode_restrictions_enabled) {
		return true;
	}
	printer_mode_t mode = get_printer_mode();
	return mode != pm_telnet;
}

#endif /* PRINTERMODE_H_ */

/*
 * LCD4DSettingsScreen.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: eai
 */

#include "LCD4DSettingsScreen.h"
#include "lcd_screens.h"
#include "utils.h"

const message_label_t SETTINGS_LABELS[] = {LANGUAGE_LABEL, NETWORK_LABEL, DISPLAY_SETTINGS_LABEL, CALIBRATION_LABEL, UPGRADE_FIRMWARE_LABEL, RESTART_PRINTER_LABEL};
#define SETTINGS_LABELS_COUNT (sizeof(SETTINGS_LABELS) / sizeof(message_label_t))

LCD4DSettingsScreen::LCD4DSettingsScreen() : LCD4DChoiceScreen(SETTINGS_LABELS_COUNT, translate(SETTINGS_LABEL)) {

}

LCD4DSettingsScreen::~LCD4DSettingsScreen() {
}

int LCD4DSettingsScreen::on_choice(int num) {
	if (SETTINGS_LABELS[num] == LANGUAGE_LABEL) {
		lcd_screens.language_screen->draw_screen();
	} else if (SETTINGS_LABELS[num] == NETWORK_LABEL) {
		lcd_screens.network_settings_screen->draw_screen();
	} else if (SETTINGS_LABELS[num] == DISPLAY_SETTINGS_LABEL) {
		lcd_screens.display_settings_screen->draw_screen();
	} else if (SETTINGS_LABELS[num] == CALIBRATION_LABEL) {
		lcd_screens.calibration_settings_screen->draw_screen();
	} else if (SETTINGS_LABELS[num] == RESTART_PRINTER_LABEL) {
		lcd_screens.generic_confirmation_screen->set_confirmation_message(translate(RESTART_PRINTER_HEADER), translate(RESTART_PRINTER_TEXT));
		lcd_screens.generic_confirmation_screen->set_ok_callback([](){
			system_reset(false);
		});
		lcd_screens.generic_confirmation_screen->draw_screen();
	} else if (SETTINGS_LABELS[num] == UPGRADE_FIRMWARE_LABEL) {
		lcd_screens.generic_confirmation_screen->set_confirmation_message(translate(UPGRADE_FIRMWARE_HEADER), translate(UPGRADE_FIRMWARE_TEXT));
		lcd_screens.generic_confirmation_screen->set_ok_callback([](){
			// Remount SD
			send_gcode("M721", &(StreamOutput::NullStream));

			// Show the previous screen. If a firmware file is found the
			// firmware upgrade screen will be displayed. Otherwise, the
			// settings screen will be shown.
			lcd_screens.settings_screen->draw_screen();

			// Send the upgrade command.
			struct SerialMessage message;
			message.message = "upgrade";
			message.stream = &(StreamOutput::NullStream);
			THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message);
		});
		lcd_screens.generic_confirmation_screen->draw_screen();
	}
	return 0;
}

const char* LCD4DSettingsScreen::get_label(int num) {
	return translate(SETTINGS_LABELS[num]);
}

LCD4DScreen& LCD4DSettingsScreen::get_back_screen() {
	return *lcd_screens.config_screen;
}

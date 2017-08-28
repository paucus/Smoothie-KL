/*
 * LCD4DDisplaySettingsScreen.cpp
 *
 *  Created on: Dec 10, 2015
 *      Author: eai
 */

#include "LCD4DDisplaySettingsScreen.h"
#include "checksumm.h"
#include "utils.h"
#include "NetworkPublicAccess.h"
#include "lcd_screens.h"
#include <stdio.h>
#include "HttpFacade.h"

LCD4DDisplaySettingsScreen::LCD4DDisplaySettingsScreen() : LCD4DChoiceScreen(LCD4D_DISPLAY_SETT_NUMBER_OF_OPTIONS, translate(DISPLAY_SETTINGS_LABEL)) {

}

LCD4DDisplaySettingsScreen::~LCD4DDisplaySettingsScreen() {
}

int LCD4DDisplaySettingsScreen::on_choice(int num) {
	if (num == 0) {
		bool energy_mode = get_public_data<bool>(lcd_module_energy_mode_checksum, 0, 0, true);
		// toggle the value
		energy_mode = !energy_mode;
		PublicData::set_value(lcd_module_energy_mode_checksum, &energy_mode);

		// Redraw item
		draw_option_item(num, false);
	}
	return 0;
}


bool LCD4DDisplaySettingsScreen::is_selectable(int num) {
	return num == 0;
};
const char* LCD4DDisplaySettingsScreen::get_label(int num) {
	// If you make string that could be longer than DISPLAY_SETTINGS_BUFF_LEN, remember to enlarge it.
	if (num == 0){
		bool energy_mode = get_public_data<bool>(lcd_module_energy_mode_checksum, 0, 0, true);
		snprintf(buff, DISPLAY_SETTINGS_BUFF_LEN, translate(LCD_ENERGY_MODE_LABEL), energy_mode?"ON":"OFF");
		buff[DISPLAY_SETTINGS_BUFF_LEN-1] = '\0'; // just in case
		return buff;
	}
	return "";
}

LCD4DScreen& LCD4DDisplaySettingsScreen::get_back_screen() {
	return *lcd_screens.settings_screen;
}


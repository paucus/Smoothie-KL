/*
 * LCD4DCalibrationSettingsScreen.cpp
 *
 *  Created on: Jan 04, 2016
 *      Author: eai
 */

#include "LCD4DCalibrationSettingsScreen.h"
#include "lcd_screens.h"

const message_label_t CALIBRATION_SETTINGS_LABELS[] = {Z_PROBE_CALIBRATION_LABEL, PID_TUNING_LABEL};
#define CALIBRATION_SETTINGS_LABELS_COUNT (sizeof(CALIBRATION_SETTINGS_LABELS) / sizeof(message_label_t))

LCD4DCalibrationSettingsScreen::LCD4DCalibrationSettingsScreen() : LCD4DChoiceScreen(CALIBRATION_SETTINGS_LABELS_COUNT, translate(CALIBRATION_LABEL)) {

}

LCD4DCalibrationSettingsScreen::~LCD4DCalibrationSettingsScreen() {
}

int LCD4DCalibrationSettingsScreen::on_choice(int num) {
	if (CALIBRATION_SETTINGS_LABELS[num] == Z_PROBE_CALIBRATION_LABEL) {
		lcd_screens.calibration_screen->draw_screen();
	} else if (CALIBRATION_SETTINGS_LABELS[num] == PID_TUNING_LABEL) {
		lcd_screens.pid_autotune_screen->draw_screen();
	}
	return 0;
}

const char* LCD4DCalibrationSettingsScreen::get_label(int num) {
	return translate(CALIBRATION_SETTINGS_LABELS[num]);
}

LCD4DScreen& LCD4DCalibrationSettingsScreen::get_back_screen() {
	return *lcd_screens.settings_screen;
}

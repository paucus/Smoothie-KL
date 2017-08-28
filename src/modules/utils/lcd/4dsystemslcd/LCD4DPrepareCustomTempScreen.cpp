/*
 * LCD4DPrepareCustomTempScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DPrepareCustomTempScreen.h"
#include "LCD4DModule.h"
#include "font_width_table.h"

void LCD4DPrepareCustomTempScreen::set_current_filament_temperatures(int f, int hotend, int bed){
	filament = f;
	current_hotend_target_temp = hotend;
	current_bed_target_temp	= bed;
}

void LCD4DPrepareCustomTempScreen::draw_temperatures() {
	char temp[9];
	snprintf(temp, 9, "%dº", current_hotend_target_temp);
	screen_print(170, 48, MEDIUM, TEXT_CENTERED, BLACK, TEMP_WIDTH(MEDIUM), temp);

	snprintf(temp, 9, "%dº", current_bed_target_temp);
	screen_print(170, 103, MEDIUM, TEXT_CENTERED, BLACK, TEMP_WIDTH(MEDIUM), temp);
}

int LCD4DPrepareCustomTempScreen::draw_screen() {
	LCD4DScreenWithBackButton::draw_screen();

	lcd->img_Disable(handle, ALL);

	screen_print(165, 48, SMALL, TEXT_RIGHT, translate(EXTRUDER_TEMPERATURE_NAME));
	screen_print(165, 103, SMALL, TEXT_RIGHT, translate(BED_TEMPERATURE_NAME));

	draw_temperatures();

	draw_lcd_images(ibHeaterTempPlus, ibPrepareCustomAccept, handle);

	return 0;
}

int LCD4DPrepareCustomTempScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {

		for (int i = ibHeaterTempPlus; i <= ibPrepareCustomAccept; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);

		}
	} else if (action == TOUCH_MOVING) { // name is misleading, it is actually the screen being touched, not necessarily moving
		switch (button) {
			case ibHeaterTempPlus:
				if (current_hotend_target_temp < get_max_hotend_temperature()) {
					current_hotend_target_temp++;
					this->draw_temperatures();
				}
				break;
			case ibHeaterTempMinus:
				if (current_hotend_target_temp > 0) {
					current_hotend_target_temp--;
					this->draw_temperatures();
				}
				break;
			case ibBedTempPlus:
				if (current_bed_target_temp < get_max_bed_temperature()) {
					current_bed_target_temp++;
					this->draw_temperatures();
				}
				break;
			case ibBedTempMinus:
				if (current_bed_target_temp > 0) {
					current_bed_target_temp--;
					this->draw_temperatures();
				}
				break;
			default:
				break;
		}
	} else if (action == TOUCH_RELEASED) {
		for (int i = ibHeaterTempPlus; i <= ibPrepareCustomAccept; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);
		}

		//TODO now, it is limited to 300 hotend and 120 bed, do we need these limits?
		switch (button) {
			case ibHeaterTempPlus:
				if (current_hotend_target_temp < get_max_hotend_temperature()) {
					current_hotend_target_temp++;
					this->draw_temperatures();
				}
				break;
			case ibHeaterTempMinus:
				if (current_hotend_target_temp > 0) {
					current_hotend_target_temp--;
					this->draw_temperatures();
				}
				break;
			case ibBedTempPlus:
				if (current_bed_target_temp < get_max_bed_temperature()) {
					current_bed_target_temp++;
					this->draw_temperatures();
				}
				break;
			case ibBedTempMinus:
				if (current_bed_target_temp > 0) {
					current_bed_target_temp--;
					this->draw_temperatures();
				}
				break;

			case ibPrepareCustomAccept:
				send_gcode_v("M104 S%d\r\n", &StreamOutput::NullStream, current_hotend_target_temp);
				send_gcode_v("M140 S%d\r\n", &StreamOutput::NullStream, current_bed_target_temp);
				if (filament>=0 && filament<=3){
					send_gcode_v("M801 F%d B%d E%d\r\n", &StreamOutput::NullStream, filament, current_bed_target_temp, current_hotend_target_temp);
					THEKERNEL->append_gcode_to_queue("M510", &StreamOutput::NullStream);
				}
				LCD4DScreen* caller_scr = this->go_to_previous_screen();
				caller_scr->on_refresh(); // we want it to refresh as soon as possible...
				break;
		}
	}
	return 0;
}

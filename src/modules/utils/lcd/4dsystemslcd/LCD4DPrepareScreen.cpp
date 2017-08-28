/*
 * LCD4DPrepareScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DPrepareScreen.h"
#include "libs/PublicData.h"
#include "TemperatureControlPublicAccess.h"
#include "FilamentTemps.h"
#include "LCD4DChangeFilamentScreen.h"
#include "LCD4DMainScreen.h"
#include "LCD4DPrepareCustomTempScreen.h"
#include "lcd_screens.h"
#include "utils.h"
#include "EndstopsPublicAccess.h"
#include "nuts_bolts.h"
#include "PrinterParking.h"

bool LCD4DPrepareScreen::should_refresh_buttons = true;

static int icustom_temp(){
	return ibETC_EN + (int)LCD4DScreen::get_language();
}

int LCD4DPrepareScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	draw_lcd_images(ibPLA, ibSwitchFilament, handle);

	lcd->img_Enable(handle, icustom_temp());
	lcd->img_ClearAttributes(handle, icustom_temp(), I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, icustom_temp(), IMAGE_INDEX, 0);
	lcd->img_Show(handle, icustom_temp());

	on_refresh();

	return 0;
}

void LCD4DPrepareScreen::on_refresh() {
	if (LCD4DModule::showing_screen == this) {

		LCD4DScreen::on_refresh();

		/* we don't refresh statuses while the buttons are touched */
		if (should_refresh_buttons) {
			if (target_bed_temperature == PLA_BED_TEMPERATURE && target_hotend_temperature == PLA_HOTEND_TEMPERATURE) {
				if (lcd->img_GetWord(handle, ibPLA, IMAGE_INDEX) != 1) {
					for (int i = ibPLA; i <= icustom_temp(); i++) {
						lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == ibPLA) ? 1 : 0);
						lcd->img_Show(handle, i);
					}
				}
			} else if (target_bed_temperature == ABS_BED_TEMPERATURE
					&& target_hotend_temperature == ABS_HOTEND_TEMPERATURE) {
				if (lcd->img_GetWord(handle, ibABS, IMAGE_INDEX) != 1) {
					for (int i = ibPLA; i <= icustom_temp(); i++) {
						lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == ibABS) ? 1 : 0);
						lcd->img_Show(handle, i);
					}
				}
			} else if (target_bed_temperature == NYLON_BED_TEMPERATURE
					&& target_hotend_temperature == NYLON_HOTEND_TEMPERATURE) {
				if (lcd->img_GetWord(handle, ibNylon, IMAGE_INDEX) != 1) {
					for (int i = ibPLA; i <= icustom_temp(); i++) {
						lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == ibNylon) ? 1 : 0);
						lcd->img_Show(handle, i);
					}
				}
			} else if (target_bed_temperature == FLEX_BED_TEMPERATURE
					&& target_hotend_temperature == FLEX_HOTEND_TEMPERATURE) {
				if (lcd->img_GetWord(handle, ibFlexible, IMAGE_INDEX) != 1) {
					for (int i = ibPLA; i <= icustom_temp(); i++) {
						lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == ibFlexible) ? 1 : 0);
						lcd->img_Show(handle, i);
					}
				}
			} else if ((target_bed_temperature != 0 || target_hotend_temperature != 0)) {
				if (lcd->img_GetWord(handle, icustom_temp(), IMAGE_INDEX) != 1) {
					for (int i = ibPLA; i <= icustom_temp(); i++) {
						lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == icustom_temp()) ? 1 : 0);
						lcd->img_Show(handle, i);
					}
				}
			} else if (target_bed_temperature == 0 && target_hotend_temperature == 0) {
				/* it is 0, check buttons... */
				for (int i = ibPLA; i <= icustom_temp(); i++) {
					lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
					lcd->img_Show(handle, i);
				}
			}
		}

	}
}

int LCD4DPrepareScreen::process_click(int action, int button) {
	static unsigned long timer = millis();
	if (action == TOUCH_PRESSED) {
		for (int i = ibPLA; i <= icustom_temp(); i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}
		should_refresh_buttons = false;
		timer = millis();

	} else if (action == TOUCH_RELEASED) {
		should_refresh_buttons = true;

		for (int i = ibPLA; i <= icustom_temp(); i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);
		}
		switch (button) {
			case ibPLA:
				if ((millis() - timer) >= 2000) {
					lcd_screens.custom_temperature_screen->set_current_filament_temperatures(PLA, PLA_HOTEND_TEMPERATURE, PLA_BED_TEMPERATURE);
					lcd_screens.custom_temperature_screen->draw_screen();
				} else {
					lcd->img_SetWord(handle, ibPLA, IMAGE_INDEX, 1);
					lcd->img_Show(handle, ibPLA);
					set_temperatures(PLA_BED_TEMPERATURE, PLA_HOTEND_TEMPERATURE);
				}
				break;
			case ibABS:
				if ((millis() - timer) >= 2000) {
					lcd_screens.custom_temperature_screen->set_current_filament_temperatures(ABS, ABS_HOTEND_TEMPERATURE, ABS_BED_TEMPERATURE);
					lcd_screens.custom_temperature_screen->draw_screen();
				} else {
					lcd->img_SetWord(handle, ibABS, IMAGE_INDEX, 1);
					lcd->img_Show(handle, ibABS);
					set_temperatures(ABS_BED_TEMPERATURE, ABS_HOTEND_TEMPERATURE);
				}
				break;
			case ibNylon:
				if ((millis() - timer) >= 2000) {
					lcd_screens.custom_temperature_screen->set_current_filament_temperatures(NYLON, NYLON_HOTEND_TEMPERATURE, NYLON_BED_TEMPERATURE);
					lcd_screens.custom_temperature_screen->draw_screen();
				} else {
					lcd->img_SetWord(handle, ibNylon, IMAGE_INDEX, 1);
					lcd->img_Show(handle, ibNylon);
					set_temperatures(NYLON_BED_TEMPERATURE, NYLON_HOTEND_TEMPERATURE);
				}
				break;
			case ibFlexible:
				if ((millis() - timer) >= 2000) {
					lcd_screens.custom_temperature_screen->set_current_filament_temperatures(FLEXIBLE, FLEX_HOTEND_TEMPERATURE, FLEX_BED_TEMPERATURE);
					lcd_screens.custom_temperature_screen->draw_screen();
				} else {
					lcd->img_SetWord(handle, ibFlexible, IMAGE_INDEX, 1);
					lcd->img_Show(handle, ibFlexible);
					set_temperatures(FLEX_BED_TEMPERATURE, FLEX_HOTEND_TEMPERATURE);
				}
				break;
			case ibCooldown:
				{
					lcd->img_SetWord(handle, ibCooldown, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibCooldown);

					static float last_heater_temp = ABS_HOTEND_TEMPERATURE;
					static float last_bed_temp = ABS_BED_TEMPERATURE;
					float heater_temp, target_heater_temp, bed_temp, target_bed_temp;
					get_temperatures(&bed_temp, &target_bed_temp, &heater_temp, &target_heater_temp);
					if (target_heater_temp == 0.0 && target_bed_temp == 0.0) {
						// It has already been cooled down.
						set_temperatures(last_bed_temp, last_heater_temp);
					} else {
						last_bed_temp = target_bed_temp;
						last_heater_temp = target_heater_temp;
						set_temperatures(0.0, 0.0);
					}

				}
				break;
			case ibSwitchFilament:
				{
					bool* axis_known_positions = get_public_data_ptr<bool>(endstops_checksum, axis_position_known_checksum, 0, nullptr);
					float* change_filament_position = get_public_data_ptr<float>(change_filament_position_checksum, 0, 0, nullptr);
					if (axis_known_positions && change_filament_position) {
						if (!axis_known_positions[X_AXIS] || !axis_known_positions[Y_AXIS]) {
							send_gcode("G28 X0 Y0", &(StreamOutput::NullStream));
						}
						send_gcode_v("G0 X%.3f Y%.3f F7000", &(StreamOutput::NullStream), change_filament_position[X_AXIS], change_filament_position[Y_AXIS]);
					}
					lcd_screens.change_filament_screen->draw_screen();
				}
				break;
			case ibETC_EN:
				lcd_screens.custom_temperature_screen->set_current_filament_temperatures(-1, target_hotend_temperature, target_bed_temperature);
				lcd_screens.custom_temperature_screen->draw_screen();
				break;
			case ibETC_SP:
				lcd_screens.custom_temperature_screen->set_current_filament_temperatures(-1, target_hotend_temperature, target_bed_temperature);
				lcd_screens.custom_temperature_screen->draw_screen();
				break;
			case ibETC_PO:
				lcd_screens.custom_temperature_screen->set_current_filament_temperatures(-1, target_hotend_temperature, target_bed_temperature);
				lcd_screens.custom_temperature_screen->draw_screen();
				break;
			case ibBackPrepare:
				lcd_screens.main_screen->draw_screen();
				break;
			default:
				break;
		}

	}

	return 0;
}

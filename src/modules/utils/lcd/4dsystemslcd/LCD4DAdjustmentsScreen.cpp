/*
 * LCD4DAdjustmentsScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DAdjustmentsScreen.h"
#include "LCD4DModule.h"

#include "LCD4DPrintingScreen.h"
#include "font_width_table.h"
#include "lcd_screens.h"

LCD4DAdjustmentsScreen::LCD4DAdjustmentsScreen(){
	field_being_changed = F_NONE;
	value_begin_changed = 0.0;
	current_speed_modifier = 100;
	current_flow_rate_modifier = 100;
}

float LCD4DAdjustmentsScreen::field_value(lcdadj_field_t field) {
	if (field == field_being_changed) {
		return value_begin_changed;
	}
	switch (field) {
		case F_HOTEND_TARGET:
			return target_hotend_temperature;
		case F_BED_TARGET:
			return target_bed_temperature;
		default:
			// unknown. something went wrong. TODO tell somewhere that this is an invalid value.
			return -1.0;
	}
}
void LCD4DAdjustmentsScreen::set_field_being_changed(lcd_image_enum_t button){
	switch (button) {
		case ibAdjustmentsExtruderMinus:
		case ibAdjustmentsExtruderPlus:
			field_being_changed = F_HOTEND_TARGET;
			value_begin_changed = target_hotend_temperature;
			break;
		case ibAdjustmentsBedTempMinus:
		case ibAdjustmentsBedTempPlus:
			field_being_changed = F_BED_TARGET;
			value_begin_changed = target_bed_temperature;
			break;
		default:
			field_being_changed = F_NONE;
	}
}
void LCD4DAdjustmentsScreen::unset_field_being_changed() {
	switch (field_being_changed) {
		case F_HOTEND_TARGET:
			target_hotend_temperature = value_begin_changed;
			break;
		case F_BED_TARGET:
			target_bed_temperature = value_begin_changed;
			break;
		default:
			break;
	}
	field_being_changed = F_NONE;
}

void LCD4DAdjustmentsScreen::print_current_values() {
	char temp[9];
	snprintf(temp, 9, "%.0fº", field_value(F_HOTEND_TARGET));
	screen_print(170, 16, MEDIUM, TEXT_CENTERED, BLACK, TEMP_WIDTH(MEDIUM), temp);
	snprintf(temp, 9, "%.0fº", field_value(F_BED_TARGET));
	screen_print(170, 58, MEDIUM, TEXT_CENTERED, BLACK, TEMP_WIDTH(MEDIUM), temp);
	snprintf(temp, 9, "%d%%", current_speed_modifier);
	screen_print(170, 98, MEDIUM, TEXT_CENTERED, BLACK, PERCENT_WIDTH(MEDIUM), temp);
	snprintf(temp, 9, "%d%%", current_flow_rate_modifier);
	screen_print(170, 138, MEDIUM, TEXT_CENTERED, BLACK, PERCENT_WIDTH(MEDIUM), temp);
}

int LCD4DAdjustmentsScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	this->get_current_speed(&current_speed_modifier);
	this->get_current_flowrate(&current_flow_rate_modifier);

	lcd->img_Disable(handle, ALL);

	screen_print(160, 16, SMALL, TEXT_RIGHT, KIKAIBLUE, translate(EXTRUDER_TEMPERATURE_NAME));
	screen_print(160, 58, SMALL, TEXT_RIGHT, KIKAIBLUE, translate(BED_TEMPERATURE_NAME));
	screen_print(160, 98, SMALL, TEXT_RIGHT, KIKAIBLUE, translate(PRINTING_SPEED_NAME));
	screen_print(160, 138, SMALL, TEXT_RIGHT, KIKAIBLUE, translate(FILAMENT_FLOW_RATE_NAME));

	// clean any previous remaining setting
	field_being_changed = F_NONE;

	print_current_values();

	draw_lcd_images(ibAdjustmentsExtruderMinus, ibAdjustmentsBack, handle);

	bool fan_on;
	this->get_fan_status(&fan_on);

	lcd->img_Enable(handle, ibAdjustmentsToggleFan);
	lcd->img_ClearAttributes(handle, ibAdjustmentsToggleFan, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, ibAdjustmentsToggleFan, IMAGE_INDEX, fan_on ? 2 : 0);
	lcd->img_Show(handle, ibAdjustmentsToggleFan);

	return 0;
}

int LCD4DAdjustmentsScreen::process_click(int action, int button) {
	static int previous_toggle_status;
	if (action == TOUCH_PRESSED) {
		for (int i = ibAdjustmentsExtruderMinus; i <= ibAdjustmentsBack; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);

		}

		// if any of the + and - buttons is selected, load its value.
		set_field_being_changed((lcd_image_enum_t)button);

		if (button == ibAdjustmentsToggleFan) {
			// Fan has 3 states... So we need to save the current one before changing...
			previous_toggle_status = lcd->img_GetWord(handle, ibAdjustmentsToggleFan, IMAGE_INDEX);

			lcd->img_SetWord(handle, ibAdjustmentsToggleFan, IMAGE_INDEX, 1);
			lcd->img_Show(handle, ibAdjustmentsToggleFan);
		}
	} else if (action == TOUCH_MOVING) { // name is misleading, it is actually the screen being touched, not necessarily moving
		switch (button) {
			case ibAdjustmentsExtruderMinus:
				//Can't go below 0
				if (value_begin_changed > 0)
					value_begin_changed--;
				break;
			case ibAdjustmentsExtruderPlus:
				//Can't go above 300
				if (value_begin_changed < get_max_hotend_temperature())
					value_begin_changed++;
				break;
			case ibAdjustmentsBedTempMinus:
				//Can't go below 0
				if (value_begin_changed > 0)
					value_begin_changed--;
				break;
			case ibAdjustmentsBedTempPlus:
				//Can't go above 300
				if (value_begin_changed < get_max_bed_temperature())
					value_begin_changed++;
				break;
			case ibAdjustmentsPrintSpeedMinus:
				//Can't go slower than 10%
				if (current_speed_modifier > 10)
					current_speed_modifier--;
				break;
			case ibAdjustmentsPrintSpeedPlus:
				//Can't go above 300%
				if (current_speed_modifier < 300)
					current_speed_modifier++;
				break;
			case ibAdjustmentsBlowerRateMinus:
				//Can't go slower than 1%
				if (current_flow_rate_modifier > 1)
					current_flow_rate_modifier--;
				break;
			case ibAdjustmentsBlowerRatePlus:
				//Can't go above 100%
				if (current_flow_rate_modifier < 300)
					current_flow_rate_modifier++;
				break;
			default:
				break;
		}

		if (button >= ibAdjustmentsExtruderMinus && button <= ibAdjustmentsBlowerRatePlus) {
			print_current_values();
		}
	} else if (action == TOUCH_RELEASED) {
		for (int i = ibAdjustmentsExtruderMinus; i <= ibAdjustmentsBack; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);

		}
		switch (button) {
			case ibAdjustmentsExtruderMinus:
				//Can't go below 0
				if (value_begin_changed > 0)
					value_begin_changed--;
				break;
			case ibAdjustmentsExtruderPlus:
				//Can't go above 300
				if (value_begin_changed < get_max_hotend_temperature())
					value_begin_changed++;
				break;
			case ibAdjustmentsBedTempMinus:
				//Can't go below 0
				if (value_begin_changed > 0)
					value_begin_changed--;
				break;
			case ibAdjustmentsBedTempPlus:
				//Can't go above 300
				if (value_begin_changed < get_max_bed_temperature())
					value_begin_changed++;
				break;
			case ibAdjustmentsPrintSpeedMinus:
				//Can't go slower than 10%
				if (current_speed_modifier > 10)
					current_speed_modifier--;
				break;
			case ibAdjustmentsPrintSpeedPlus:
				if (current_speed_modifier < 300)
					current_speed_modifier++;
				break;
			case ibAdjustmentsBlowerRateMinus:
				//Can't go slower than 1%
				if (current_flow_rate_modifier > 1)
					current_flow_rate_modifier--;
				break;
			case ibAdjustmentsBlowerRatePlus:
				//Can't go above 100%
				if (current_flow_rate_modifier < 300)
					current_flow_rate_modifier++;
				break;
			case ibAdjustmentsToggleFan:
				if (previous_toggle_status == 0) {
					/* Need to start fan */
					lcd->img_SetWord(handle, ibAdjustmentsToggleFan, IMAGE_INDEX, 2);
					lcd->img_Show(handle, ibAdjustmentsToggleFan);

					THEKERNEL->append_gcode_to_queue("M106 S255", &StreamOutput::NullStream);
				} else {
					/* Need to stop fan */
					lcd->img_SetWord(handle, ibAdjustmentsToggleFan, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibAdjustmentsToggleFan);

					THEKERNEL->append_gcode_to_queue("M106 S0", &StreamOutput::NullStream);
				}
				break;

			case ibAdjustmentsBack:
				lcd_screens.printing_screen->draw_screen();
				break;
			default:
				break;
		}

		// Here it unsets the current field and updates the real field with the modified value.
		unset_field_being_changed();

		//redraw and variable changing was done already... now send the commands...
		if (button == ibAdjustmentsPrintSpeedMinus || button == ibAdjustmentsPrintSpeedPlus) {
			int new_speed_override = min(300, current_speed_modifier);
			THEKERNEL->append_gcode_v_to_queue("M220 S%d", &StreamOutput::NullStream, new_speed_override);
		}
		//redraw and variable changing was done already... now send the commands...
		if (button == ibAdjustmentsBlowerRateMinus || button == ibAdjustmentsBlowerRatePlus) {
			int new_flowrate_override = min(300, current_flow_rate_modifier);
			THEKERNEL->append_gcode_v_to_queue("M221 S%d", &StreamOutput::NullStream, new_flowrate_override);
		}

		if (button >= ibAdjustmentsExtruderMinus && button <= ibAdjustmentsBedTempPlus) {
			set_temperatures(target_bed_temperature, target_hotend_temperature);
		}

		if (button >= ibAdjustmentsExtruderMinus && button <= ibAdjustmentsBlowerRatePlus) {
			print_current_values();
		}
	}

	return 0;
}

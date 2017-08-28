/*
 * LCD4DManualControlScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DManualControlScreen.h"
#include "LCD4DModule.h"
#include "FilamentTemps.h"
#include "LCD4DMainScreen.h"
#include "PublicData.h"
#include "Robot.h"
#include "checksumm.h"
#include "nuts_bolts.h"
#include "modules/tools/extruder/ExtruderPublicAccess.h"
#include "lcd_screens.h"

#include "RobotConsts.h"
#define Z_FEEDRATE_MANUAL_CONTROL ((float)Z_MAX_SPEED)

static bool is_button_pressed = false;
static int button_pressed;
static float current_pos[3] = {0,0,0};
static float current_e_pos = 0.0;
static unsigned long current_timestamp;

static void get_current_pos(float *cp) {

	THEKERNEL->robot->get_axis_position(cp);
}

static void get_current_e(float *ce){
	float result;
	if (!PublicData::get_value( extruder_checksum, target_position_checksum, &result)){
		*ce = 0.0;
		return;
	}
	*ce = result;
}

void LCD4DManualControlScreen::on_refresh() {
	LCD4DScreen::on_refresh();
}

void LCD4DManualControlScreen::on_periodic_tick(){

	if(is_button_pressed){
		int current_speed_modifier_percentage;
		this->get_current_speed(&current_speed_modifier_percentage);
		switch (button_pressed) {
			case ibManualZDown:
			case ibManualZUp:
			{
				bool move_up = (button_pressed == ibManualZUp);
				float target_pos = current_pos[2] + (move_up?-1.0:1.0) * (millis() + 100 - current_timestamp) * Z_FEEDRATE_MANUAL_CONTROL / MILLISECONDS_PER_MINUTE;
				send_gcode_v("G1 Z%f F%f", &StreamOutput::NullStream, target_pos, (Z_FEEDRATE_MANUAL_CONTROL * 100) / current_speed_modifier_percentage);
				// disable ibManualZDown and ibManualZUp depending on the current Z position
				bool disable_up = target_pos <= 0;
				bool disable_down = target_pos >= THEKERNEL->robot->max_coord_value[Z_AXIS];

				lcd->img_SetWord(handle, ibManualZUp, IMAGE_INDEX, disable_up?2:0);
				lcd->img_Show(handle, ibManualZUp);
				lcd->img_SetWord(handle, ibManualZDown, IMAGE_INDEX, disable_down?2:0);
				lcd->img_Show(handle, ibManualZDown);
			}
			break;
		}
	}

}


int LCD4DManualControlScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	/* print top header */
	lcd->gfx_RectangleFilled(0, 0, 320, 16, KIKAIBLUE);

	screen_print(54, 1, SMALL, TEXT_CENTERED, KIKAIBLUE, WHITE, true, TEXT_CENTER_AROUND_X,
			translate(EXTRUDER_ACTION_NAME));
	screen_print(162, 1, SMALL, TEXT_CENTERED, KIKAIBLUE, WHITE, true, TEXT_CENTER_AROUND_X,
			translate(HOME_ACTION_NAME));
	screen_print(266, 1, SMALL, TEXT_CENTERED, KIKAIBLUE, WHITE, true, TEXT_CENTER_AROUND_X,
			translate(PLATFORM_ACTION_NAME));

	lcd->img_Disable(handle, ALL);

	draw_lcd_images(ibManualRetract, ibManualBack, handle);

	/* It is a toggle, so it has 4 states. target_hotend_temp is used since it is set to non-zero when heating and 0 when not*/
	lcd->img_Enable(handle, ibManualToggleTemperature);
	lcd->img_ClearAttributes(handle, ibManualToggleTemperature, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, ibManualToggleTemperature, IMAGE_INDEX, target_hotend_temperature != 0 ? 0 : 2);
	lcd->img_Show(handle, ibManualToggleTemperature);

	LCD4DScreen::on_refresh();

	// disable ibManualZDown and ibManualZUp depending on the current Z position
	float pos[3];
	get_current_pos(pos);
	if (pos[Z_AXIS] <= 0) {
		// disable up
		lcd->img_SetWord(handle, ibManualZUp, IMAGE_INDEX, 2);
		lcd->img_Show(handle, ibManualZUp);
	} else if (pos[Z_AXIS] >= THEKERNEL->robot->max_coord_value[Z_AXIS]){
		//disable down
		lcd->img_SetWord(handle, ibManualZDown, IMAGE_INDEX, 2);
		lcd->img_Show(handle, ibManualZDown);
	}

	return 0;
}

int LCD4DManualControlScreen::process_click(int action, int button) {

	if (action == TOUCH_PRESSED) {

		for (int i = ibManualRetract; i <= ibManualBack; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}

		/* It is a toggle, so it has 4 states. target_hotend_temp is used since it is set to non-zero when heating and 0 when not*/
		if (button == ibManualToggleTemperature) {
			lcd->img_SetWord(handle, ibManualToggleTemperature, IMAGE_INDEX, target_hotend_temperature != 0 ? 1 : 3);
		} else {
			lcd->img_SetWord(handle, ibManualToggleTemperature, IMAGE_INDEX, target_hotend_temperature != 0 ? 0 : 2);
		}
		lcd->img_Show(handle, ibManualToggleTemperature);

		current_timestamp = millis();
		get_current_pos(current_pos);
		get_current_e(&current_e_pos);

		switch (button) {
			case ibManualZDown:
				send_gcode("G90", &StreamOutput::NullStream);
				is_button_pressed = true;
				break;
			case ibManualZUp:
				send_gcode("G90", &StreamOutput::NullStream);
				is_button_pressed = true;
				break;
		}

		button_pressed = button;

	} else if (action == TOUCH_RELEASED) {
		// We will use the current position to decide whether we must gray out ibManualZDown and
		// ibManualZUp. We mustn't update current_pos, because current_pos[Z_AXIS] is used as the
		// historical position where the movement started in the timer event.
		float pos[3];	// temp var to hold the current position
		get_current_pos(pos);

		for (int i = ibManualRetract; i <= ibManualBack; i++) {
			if (i == ibManualZDown){
				bool disable_down = pos[Z_AXIS] >= THEKERNEL->robot->max_coord_value[Z_AXIS];
				lcd->img_SetWord(handle, i, IMAGE_INDEX, disable_down?2:0);
			} else if (i == ibManualZUp){
				bool disable_up = pos[Z_AXIS] <= 0;
				lcd->img_SetWord(handle, i, IMAGE_INDEX, disable_up?2:0);
			} else {
				lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			}
			lcd->img_Show(handle, i);
		}

		switch (button) {
			case ibManualExtrude:
				THEKERNEL->append_gcode_to_queue("G91", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M744", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G1 E10.0 F200.0", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M743", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G90", &StreamOutput::NullStream);
				break;
			case ibManualRetract:
				THEKERNEL->append_gcode_to_queue("G91", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M744", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G1 E-10.0 F200.0", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M743", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G90", &StreamOutput::NullStream);
				break;
			case ibManualZDown:
				is_button_pressed = false;
				break;
			case ibManualZUp:
				is_button_pressed = false;
				break;
			case ibManualZHome:
				THEKERNEL->append_gcode_to_queue("G28 Z0.0", &StreamOutput::NullStream);
				break;
			case ibManualXYHome:
				THEKERNEL->append_gcode_to_queue("G28 X0.0 Y0.0", &StreamOutput::NullStream);
				break;
			case ibManualBack:
				lcd_screens.main_screen->draw_screen();
				break;
			case ibManualToggleTemperature:
				lcd->img_SetWord(handle, ibManualToggleTemperature, IMAGE_INDEX, target_hotend_temperature == 0 ? 0 : 2);
				lcd->img_Show(handle, ibManualToggleTemperature);
				if (target_hotend_temperature == 0) {
					// turn on
					set_temperatures(ABS_BED_TEMPERATURE, ABS_HOTEND_TEMPERATURE);
				} else {
					set_temperatures(0.0F, 0.0F);
				}
				LCD4DScreen::on_refresh();
				break;
		}
	}
	return 0;
}

/*
 * LCD4DChangeFilamentScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DChangeFilamentScreen.h"
#include "LCD4DModule.h"
#include "LCDTranslations.h"
#include "checksumm.h"
#include "modules/tools/extruder/ExtruderPublicAccess.h"
#include "Robot.h"
#include "PublicData.h"

bool LCD4DChangeFilamentScreen::was_called_from_eof_event = false;

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

void LCD4DChangeFilamentScreen::on_periodic_tick(){

	if(is_button_pressed){
		int current_speed_modifier_percentage;
		this->get_current_speed(&current_speed_modifier_percentage);
		switch (button_pressed) {
			case ibChangeFilamentZDown:
				send_gcode_v("G1 Z%f F%f", &StreamOutput::NullStream, current_pos[2] + (millis() + 100 - current_timestamp) * Z_FEEDRATE_MANUAL_CONTROL / MILLISECONDS_PER_MINUTE , (Z_FEEDRATE_MANUAL_CONTROL * 100) / current_speed_modifier_percentage);
			break;
			case ibChangeFilamentZUp:
				send_gcode_v("G1 Z%f F%f", &StreamOutput::NullStream, current_pos[2] - (millis() + 100 - current_timestamp) * Z_FEEDRATE_MANUAL_CONTROL / MILLISECONDS_PER_MINUTE , (Z_FEEDRATE_MANUAL_CONTROL * 100) / current_speed_modifier_percentage);
			break;
		}
	}

}

void LCD4DChangeFilamentScreen::on_refresh() {
	LCD4DScreen::on_refresh();
}

int LCD4DChangeFilamentScreen::draw_screen() {
	LCD4DScreenWithBackButton::draw_screen();

	screen_print(10, 0, MEDIUM, TEXT_LEFT, translate(CHANGE_FILAMENT_LABEL));

	screen_print(12, 19, SMALL, TEXT_LEFT, translate(CHANGE_FILAMENT_TEXT));

	screen_print(215, 112, SMALL, TEXT_LEFT, translate(PLATFORM_ACTION_NAME));
	screen_print(52, 112, SMALL, TEXT_LEFT, translate(EXTRUDER_ACTION_NAME));

	lcd->img_Disable(handle, ALL);

	draw_lcd_images(ibFilamentExtrude, ibChangeFilamentZDown, handle);

	return 0;
}

int LCD4DChangeFilamentScreen::process_click(int action, int button) {


	if (action == TOUCH_PRESSED) {

		for (int i = ibFilamentExtrude; i <= ibChangeFilamentZDown; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}

		current_timestamp = millis();
		get_current_pos(current_pos);
		get_current_e(&current_e_pos);

		switch (button) {
			case ibChangeFilamentZDown:
				send_gcode("G90", &StreamOutput::NullStream);
				is_button_pressed = true;
				break;
			case ibChangeFilamentZUp:
				send_gcode("G90", &StreamOutput::NullStream);
				is_button_pressed = true;
				break;
		}

		button_pressed = button;

	} else if (action == TOUCH_RELEASED) {

		for (int i = ibFilamentExtrude; i <= ibChangeFilamentZDown; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);
		}

		switch (button) {
			case ibFilamentExtrude:
				THEKERNEL->append_gcode_to_queue("G91", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M744", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G1 E10.0 F320.0", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M743", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G90", &StreamOutput::NullStream);

				break;
			case ibFilamentRetract:
				THEKERNEL->append_gcode_to_queue("G91", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M744", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G1 E-10.0 F320.0", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("M743", &StreamOutput::NullStream);
				THEKERNEL->append_gcode_to_queue("G90", &StreamOutput::NullStream);

				break;
			case ibChangeFilamentZDown:
				is_button_pressed = false;
				break;
			case ibChangeFilamentZUp:
				is_button_pressed = false;
				break;
			case ibDoneChange:
				// Resume print
				if (was_called_from_eof_event) {
					THEKERNEL->append_gcode_to_queue("M824 F1", &StreamOutput::NullStream);
					was_called_from_eof_event = false;
				}

				this->go_to_previous_screen();

				break;
				default:
				break;
			}
		}
	return 0;
}

/*
 * LCD4DPIDAutotuneScreen.cpp
 *
 *  Created on: Aug 26, 2015
 *      Author: eai
 */

#include "LCD4DPIDAutotuneScreen.h"
#include "LCD4DModule.h"
#include "TemperatureControl.h"
#include "GcodeUtils.h"
#include "LCD4DCalibrationSettingsScreen.h"
#include "utils.h"
#include "font_width_table.h"
#include "Pauser.h"
#include "PublicData.h"
#include "StreamOutputPool.h"
#include "Uptime.h"
#include "lcd_screens.h"

#define WS_CHOOSE 0
#define WS_BEGIN 1
#define WS_TUNED 2
#define WS_TESTING 3
#define WS_TESTED 4
#define LABELS_RIGHT_SPACING	160
#define TEXTS_RIGHT_SPACING		170
#define EXTRUDER_MEASURE_COUNT "8"
#define EXTRUDER_MEASURE_TEMP "150"
#define BED_MEASURE_COUNT "8"
#define BED_MEASURE_TEMP "55"

LCD4DPIDAutotuneScreen::LCD4DPIDAutotuneScreen() {
	wizard_step = WS_CHOOSE;
	pool_index = 0;
	temp = 0;
	peak_temp = 0;
	time_to_stabilize = 0;
	time_to_reach_temp = 0;
	name_checksum = 0;
	p = i = d = 0;
}

LCD4DPIDAutotuneScreen::~LCD4DPIDAutotuneScreen() {
}

int LCD4DPIDAutotuneScreen::draw_screen(){
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	switch (wizard_step) {
		case WS_CHOOSE:
			screen_print(0, 10, MEDIUM, TEXT_LEFT, translate(CHOOSE_TEMP_CONTROL_FOR_PID_LABEL));
			draw_lcd_images(ipidbCancelBig, ipidbCancelBig, handle);
			draw_lcd_images(ipidbExtruder, ipidbHeatbed, handle);
			break;
		case WS_BEGIN:
			screen_print(0, 85, BIG, TEXT_CENTERED, translate(PIDAUTOTUNING_LABEL));

			draw_lcd_images(ipidbCancelBig, ipidbCancelBig, handle);
			break;
		case WS_TUNED:
			screen_print(0, 85, BIG, TEXT_CENTERED, translate(TEST_PIDAUTOTUNE_LABEL));

			draw_lcd_images(ipidbCancelSmall, ipidbAccept, handle);
			break;
		case WS_TESTING:
			screen_print(0, 85, BIG, TEXT_CENTERED, translate(PIDTESTING_LABEL));

			draw_lcd_images(ipidbCancelBig, ipidbCancelBig, handle);
			break;
		case WS_TESTED:
			screen_print(0, 20, BIG, TEXT_CENTERED, translate(PIDTEST_RESULT_LABEL));
			screen_print(LABELS_RIGHT_SPACING, 45, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(PEAK_TEMP_LABEL));
			screen_print(LABELS_RIGHT_SPACING, 65, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(AVG_TEMP_LABEL));
			screen_print(LABELS_RIGHT_SPACING, 85, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(TIME_TO_STABILIZE_TEMP_LABEL));
			screen_print(LABELS_RIGHT_SPACING, 105, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(TIME_TO_REACH_TEMP_LABEL));
			screen_print(10, 130, MEDIUM, TEXT_CENTERED, translate(SAVEPID_LABEL));

			char tempstr[9];
			snprintf(tempstr, sizeof(tempstr), "%.1fº", peak_temp);
			screen_print(TEXTS_RIGHT_SPACING, 45, MEDIUM, TEXT_LEFT, WHITE, BLACK, TEMP_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, tempstr);
			snprintf(tempstr, sizeof(tempstr), "%.1fº", temp);
			screen_print(TEXTS_RIGHT_SPACING, 65, MEDIUM, TEXT_LEFT, WHITE, BLACK, TEMP_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, tempstr);
			long hours;
			int minutes, seconds;
			convert_to_time_units(time_to_stabilize, &hours, &minutes, &seconds);
			format_time(tempstr, hours, minutes, seconds);
			screen_print(TEXTS_RIGHT_SPACING, 85, MEDIUM, TEXT_LEFT, WHITE, BLACK, HOUR_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, trim_left_cstr(tempstr));
			convert_to_time_units(time_to_reach_temp, &hours, &minutes, &seconds);
			format_time(tempstr, hours, minutes, seconds);
			screen_print(TEXTS_RIGHT_SPACING, 105, MEDIUM, TEXT_LEFT, WHITE, BLACK, HOUR_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, trim_left_cstr(tempstr));

			draw_lcd_images(ipidbCancelSmall, ipidbAccept, handle);
	}
	return 0;
}

int LCD4DPIDAutotuneScreen::savePIDValues(int pool_index){
	void *returned_data;
	bool ok = PublicData::get_value( temperature_control_checksum, pool_index_checksum, pool_index, &returned_data );
	if (ok) {
		TemperatureControl* temp_control = *static_cast<TemperatureControl **>(returned_data);
		// Keep values to restore them in case something fails
		p = temp_control->p_factor;
		i = temp_control->i_factor;
		d = temp_control->d_factor;
		THEKERNEL->streams->printf("PID values P=%.3f I=%.3f D=%.3f saved\n", p, i, d);
		return 0;
	} else {
		THEKERNEL->streams->printf("Failed to get %s temperature control\n", pool_index==0?"extruder":"heatbed");
		return -1;
	}
}
int LCD4DPIDAutotuneScreen::restorePIDValues(int pool_index){
	void *returned_data;
	bool ok = PublicData::get_value( temperature_control_checksum, pool_index_checksum, pool_index, &returned_data );
	if (ok) {
		TemperatureControl* temp_control = *static_cast<TemperatureControl **>(returned_data);
		// Keep values to restore them in case something fails
		temp_control->p_factor = p;
		temp_control->i_factor = i;
		temp_control->d_factor = d;
		THEKERNEL->streams->printf("PID values restored to P=%.3f I=%.3f D=%.3f\n", p, i, d);
		return 0;
	} else {
		THEKERNEL->streams->printf("Failed to get %s temperature control\n", pool_index==0?"extruder":"heatbed");
		return -1;
	}
}

int LCD4DPIDAutotuneScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		for (int i = ipidbCancelBig; i <= ipidbHeatbed; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ipidbExtruder:
				if (savePIDValues(0) != 0){
					return 0;
				}

				// TODO make it configurable
				send_gcode("M303 E0 C" EXTRUDER_MEASURE_COUNT " S" EXTRUDER_MEASURE_TEMP, &(StreamOutput::NullStream));
				break;
			case ipidbHeatbed:
				if (savePIDValues(1) != 0){
					return 0;
				}

				// TODO make it configurable
				send_gcode("M303 E1 C" BED_MEASURE_COUNT " S" BED_MEASURE_TEMP, &(StreamOutput::NullStream));
				break;

			case ipidbCancelBig:
				if (wizard_step == WS_BEGIN) {
					// Cancel autotuning. In this case this button is only available in WS_BEGIN, so we don't need to test the wizard_step.
					send_gcode("M304", &(StreamOutput::NullStream));
				} else if (wizard_step == WS_TESTING) {
					// Cancel test
					send_gcode("M804", &(StreamOutput::NullStream));
				} else if (wizard_step == WS_CHOOSE) {
					lcd_screens.calibration_settings_screen->draw_screen();
				}
				break;
			case ipidbCancelSmall:
				if (wizard_step == WS_TUNED) {
					// Don't test, but give the option to save these values anyway. If the user
					// wants to restore the previous values, he will have the chance in the next
					// screen.
					on_pid_tested(0, 0, 0, 0);
				} else if(wizard_step == WS_TESTED) {
					restorePIDValues(pool_index);
					lcd_screens.calibration_settings_screen->draw_screen();
				}
				break;
			case ipidbAccept:
				if (wizard_step == WS_TUNED){
					wizard_step = WS_TESTING;
					this->draw_screen();

					send_gcode_v("M803 E%d S%.2f", &(StreamOutput::NullStream), (int)pool_index, temp);
				} else if (wizard_step == WS_TESTED) {
					THEKERNEL->append_gcode_to_queue("M500", &(StreamOutput::NullStream));	// (can't send_gcode, as it doesn't call a ON_CONSOLE_LINE_RECEIVED event)
					// This is the last step
					lcd_screens.calibration_settings_screen->draw_screen();
				}
				break;
		}
	}
	return 0;
}

void LCD4DPIDAutotuneScreen::reset_wizard(int pool_index, int name_checksum, float temp) {
	wizard_step = WS_BEGIN;
	this->pool_index = pool_index;
	this->name_checksum = name_checksum;
	this->temp = temp;
}

void LCD4DPIDAutotuneScreen::on_finish_autotune() {
#ifdef NO_UTILS_PIDTEST
	// No test, go on to the last step with fake data
	on_pid_tested(0, 0, 0, 0);
#else
	wizard_step = WS_TUNED;
	draw_screen();
#endif // NO_UTILS_PIDTEST
}

void LCD4DPIDAutotuneScreen::on_pid_tested(float peak_temp, float stable_temp, unsigned int time_to_stabilize, unsigned int time_to_reach_temp){
	wizard_step = WS_TESTED;
	this->peak_temp = peak_temp;
	this->temp = stable_temp;
	this->time_to_reach_temp = time_to_reach_temp;
	this->time_to_stabilize = time_to_stabilize;
	draw_screen();
}


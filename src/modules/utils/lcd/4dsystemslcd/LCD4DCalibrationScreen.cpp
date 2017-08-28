/*
 * LCD4DCalibrationScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DCalibrationScreen.h"
#include "LCD4DModule.h"
#include "FilamentTemps.h"
#include "LCD4DMainScreen.h"
#include "LCD4DPIDAutotuneScreen.h"
#include "lcd_screens.h"

#define NO_OF_MEASURES 5 // remember to change array initializers if you change this...

/* Defines steps and {heater, bed} temperatures*/
// In this case 100 degrees does not
static int temperatures[NO_OF_MEASURES][2] = { { 40, 40 }, { PLA_HOTEND_TEMPERATURE, PLA_BED_TEMPERATURE }, { PLA_HOTEND_TEMPERATURE, 75 }, { NYLON_HOTEND_TEMPERATURE, 90 }, { NYLON_HOTEND_TEMPERATURE, NYLON_BED_TEMPERATURE } };
static double measured[NO_OF_MEASURES] = { 0, 0, 0, 0, 0 };

// we havent started yet
static int current_step = -1;
static double compensation = 0.0;

void LCD4DCalibrationScreen::print_compensation_value() {
	char zeta[10];
	snprintf(zeta, 9, "Z: %2.1f", compensation);
	screen_print(220, 110, BIG, TEXT_LEFT, WHITE, BLACK, (word)100, true, zeta);
}

void LCD4DCalibrationScreen::on_refresh() {
	//only after passcode...
	if (current_step >= 0) {
		print_calibration_current_temperatures();
		print_compensation_value();
	}
}

void LCD4DCalibrationScreen::draw_step_screen() {

	lcd->img_Disable(handle, ALL);

	char header[100];
	snprintf(header, 99, translate(CALIBRATION_STEP_LABEL), current_step + 1, NO_OF_MEASURES,
			temperatures[current_step][0], temperatures[current_step][1]);
	screen_print(160, 10, MEDIUM, TEXT_CENTERED, WHITE, KIKAIBLUE, false, TEXT_CENTER_AROUND_X, header);

	draw_lcd_images(ibCalibrationUp, ibCalibrationDown, handle);

	int upTo = (current_step == (NO_OF_MEASURES - 1)) ? ibCalibrationDone : ibCalibrationNext;
	lcd->img_Enable(handle, upTo);
	lcd->img_ClearAttributes(handle, upTo, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, upTo, IMAGE_INDEX, 0);
	lcd->img_Show(handle, upTo);
}

void LCD4DCalibrationScreen::go_to_next_step() {
	if (++current_step == 0) {
		// send the startup commands...
		THEKERNEL->append_gcode_to_queue("M737 C1", &StreamOutput::NullStream);// Clean previous compensation values
		THEKERNEL->append_gcode_to_queue("M740 C1", &StreamOutput::NullStream);// home at center of bed
		THEKERNEL->append_gcode_to_queue("G28", &StreamOutput::NullStream); // home
		THEKERNEL->append_gcode_to_queue("G91", &StreamOutput::NullStream);// Use relative mode
		THEKERNEL->append_gcode_to_queue("G92 Z20", &StreamOutput::NullStream);// home at center of bed
	} else {
		//save the compensation value for the previous step...
		measured[current_step -1] = compensation;
		compensation = 0.0;
	}

	THEKERNEL->append_gcode_v_to_queue("M104 S%d", &StreamOutput::NullStream, temperatures[current_step][0]); // set temp and wait
	THEKERNEL->append_gcode_v_to_queue("M190 S%d", &StreamOutput::NullStream, temperatures[current_step][1]); // set temp and wait
	THEKERNEL->append_gcode_to_queue("G28 Z0 L0", &StreamOutput::NullStream);// home no autolevel
	THEKERNEL->append_gcode_to_queue("G92 Z20", &StreamOutput::NullStream);// home at center of bed

	draw_step_screen();
}

int LCD4DCalibrationScreen::draw_screen() {
	bool was_autolevel_shown = (lcd_screens.autoleveling_screen.is_ptr(LCD4DModule::showing_screen));

	LCD4DScreen::draw_screen();
	if (was_autolevel_shown) {
		draw_step_screen();
		return 0;
	}

	/* reset static vars in case we are reentering */
	compensation = 0.0;
	current_step = -1;

	lcd->img_Disable(handle, ALL);

	// Draw initial screen

	screen_print(10, 0, MEDIUM, TEXT_LEFT, KIKAIBLUE, translate(CALIBRATION_DESCRIPTION_LABEL));
	draw_lcd_images(ibCalibrationNext, ibCalibrationNext, handle);

	return 0;
}

int LCD4DCalibrationScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		for (int i = ibCalibrationUp; i <= ibCalibrationDone; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}
	} else if (action == TOUCH_RELEASED) {
		for (int i = ibCalibrationUp; i <= ibCalibrationDone; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);
		}

		float bed_delta_temp_enable = 5.0;
		float hotend_delta_temp_enable = 10.0;
		bool should_enable_move = last_checked_bed_temperature > (target_bed_temperature - bed_delta_temp_enable)
				&& last_checked_bed_temperature < (target_bed_temperature + bed_delta_temp_enable)
				&& last_checked_hotend_temp > (target_hotend_temperature - hotend_delta_temp_enable)
				&& last_checked_hotend_temp < (target_hotend_temperature + hotend_delta_temp_enable);

		switch (button) {
			case ibCalibrationUp:
				if (should_enable_move) {
					compensation -= 0.1;
					THEKERNEL->append_gcode_to_queue("G1 Z-0.1", &StreamOutput::NullStream);
					print_compensation_value();
				}
				break;
			case ibCalibrationDown:
				if (should_enable_move) {
					compensation += 0.1;
					THEKERNEL->append_gcode_to_queue("G1 Z0.1", &StreamOutput::NullStream);
					print_compensation_value();
				}
				break;
			case ibCalibrationNext:
				if (current_step < 0) {
					go_to_next_step();
				} else if (should_enable_move) {
					THEKERNEL->append_gcode_to_queue("G1 Z10", &StreamOutput::NullStream); // up 10 mm before next step...
					go_to_next_step();
				}
				break;
			case ibCalibrationDone:
				if ( should_enable_move ) {
					measured[NO_OF_MEASURES -1] = compensation;

					THEKERNEL->append_gcode_to_queue("M740 C0", &StreamOutput::NullStream);// remove the home at center of bed
					/* We are done, send all commands to save calibration... */
					for (int i = 0; i < NO_OF_MEASURES; ++i) {
						THEKERNEL->append_gcode_v_to_queue("M737 S%d Z%.2f", &StreamOutput::NullStream, temperatures[i][1], -measured[i]); // set temp and wait
					}
					THEKERNEL->append_gcode_to_queue("M510", &StreamOutput::NullStream); // save to config
					THEKERNEL->append_gcode_to_queue("G90", &StreamOutput::NullStream);// back to absolut mode

					lcd_screens.main_screen->draw_screen();
				}
				break;
		}
	}
	return 0;
}

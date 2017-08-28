/*
 * LCD4DMessageScreen.cpp
 *
 *  Created on: Mar 2, 2015
 *      Author: abialet
 */

#include "LCD4DMessageScreen.h"
#include "LCD4DModule.h"
#include <string.h>
#include <stdlib.h>

int LCD4DMessageScreen::counter = 0;

void LCD4DMessageScreen::on_refresh() {
	counter++;

	if (counter >= 10) {
		counter = 0;

		go_to_previous_screen();
	}
}

LCD4DMessageScreen::~LCD4DMessageScreen() {
	if (this->message) {
		free(this->message);
		this->message = NULL;
	}
}
void LCD4DMessageScreen::set_message(const char* message) {
	if (this->message) {
		free(this->message);
		this->message = NULL;
	}

	this->message = strdup(message);
}

int LCD4DMessageScreen::draw_screen() {
	LCD4DScreenWithBackButton::draw_screen();

	counter = 0; // reset the timer if we are asked to redraw

	if (message == NULL) {
		go_to_previous_screen();

		return 1;
	}

	lcd->img_Disable(handle, ALL);

	screen_print(0, 10, MEDIUM, TEXT_CENTERED, BLACK, true, message);

	draw_lcd_images(ibPrepareCustomAccept, ibPrepareCustomAccept, handle);

	return 0;
}

int LCD4DMessageScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {

		for (int i = ibPrepareCustomAccept; i <= ibPrepareCustomAccept; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}

	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			default:
				counter = 0;

				go_to_previous_screen();
				break;
		}
	}
	return 0;
}

void LCD4DMessageScreen::warn_cold_extrusion() {
	this->set_message(translate(COLD_EXTRUSION_PREVENTED_TEXT));
	this->draw_screen();
}
void LCD4DMessageScreen::warn_mintemp_triggered(enum mt_alrt_src_e source) {
	if (source == MT_ALRT_SRC_HEATBED) {
		this->set_message(translate(MINTEMP_HEATBED_TEXT));
	} else if (source == MT_ALRT_SRC_HOTEND) {
		this->set_message(translate(MINTEMP_HOTEND_TEXT));
	} else {
		this->set_message(translate(MINTEMP_TEXT));
	}
	this->draw_screen();
}
void LCD4DMessageScreen::warn_thermistor_out() {
	this->set_message(translate(THERMISTOR_OUT_TEXT));
	this->draw_screen();
}
void LCD4DMessageScreen::warn_unknown_error(const char* msg) {
	this->set_message(msg);
	this->draw_screen();
}

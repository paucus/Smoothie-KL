/*
 * LCD4DPrintDoneScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DPrintDoneScreen.h"
#include "LCD4DModule.h"

#include "LCD4DSelectModelScreen.h"
#include "utils.h"
#include "font_width_table.h"
#include "lcd_screens.h"
#include "PrinterMode.h"

#define MIN_TEMP_TO_TOUCH 60
#define TEXTS_RIGHT_SPACING	10

static bool clear_screen = true;

void LCD4DPrintDoneScreen::on_refresh() {
	LCD4DScreen::on_refresh();
	screen_print(0, 115, MEDIUM, TEXT_CENTERED, BLACK, translate(PRINTING_SCREEN_ELAPSED));
	screen_print(0, 140, MEDIUM, TEXT_CENTERED, KIKAIBLUE, HOUR_WIDTH(MEDIUM), this->total_print_time_str);

	if (last_checked_bed_temperature > MIN_TEMP_TO_TOUCH || last_checked_hotend_temp > MIN_TEMP_TO_TOUCH) {
		screen_print(0, 65, MEDIUM, TEXT_CENTERED, BLACK, translate(PRINTING_DONE_WAIT_TEXT));
	} else {
		if (clear_screen){
			lcd->gfx_RectangleFilled(0, 65, 320, 175, WHITE);
			clear_screen = false;
		}
		screen_print(0, 65, MEDIUM, TEXT_CENTERED, BLACK, translate(PRINTING_DONE_REMOVE_TEXT));
	}


}

int LCD4DPrintDoneScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	draw_lcd_images(ibPrepareCustomAccept, ibPrepareCustomAccept, handle);

	screen_print(0, 20, BIG, TEXT_CENTERED, KIKAIBLUE, translate(PRINTING_DONE_HEADER));

	on_refresh();

	return 0;
}

int LCD4DPrintDoneScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		for (int i = ibPrepareCustomAccept; i <= ibPrepareCustomAccept; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);

		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ibPrepareCustomAccept:
				if (sd_print_allowed()){
					lcd_screens.select_model_screen->draw_screen();
				} else {
					// Probably USB mode was enabled when print finished.
					// To prevent misunderstandings, go to the main screen.
					lcd_screens.main_screen->draw_screen();
				}
				clear_screen = true; // we need to make room for text... set the flag on exit
				break;
			default:
				break;
		}
	}
	return 0;
}

void LCD4DPrintDoneScreen::set_print_time(unsigned int print_secs) {
	long int hs;
	int ms, ss;
	convert_to_time_units(print_secs, &hs, &ms, &ss);
	format_time(total_print_time_str, hs, ms, ss);
}

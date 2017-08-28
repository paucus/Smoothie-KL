/*
 * LCD4DPauseScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DPauseScreen.h"
#include "LCD4DModule.h"

#include "LCD4DPrintingScreen.h"
#include "LCD4DChangeFilamentScreen.h"
#include "lcd_screens.h"

int LCD4DPauseScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

//	lcd->img_Enable(handle, iPauseImage);
//	lcd->img_Show(handle, iPauseImage);

	screen_print(10, 25, BIG, TEXT_CENTERED, KIKAIBLUE, translate(PAUSED_PRINT_HEADER));
	screen_print(10, 70, MEDIUM, TEXT_LEFT, BLACK, true, translate(PAUSED_PRINT_TEXT));

	draw_lcd_images(ibPauseBack, ibPauseSwitchFilament, handle);

	return 0;
}

int LCD4DPauseScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {

		for (int i = ibPauseBack; i <= ibPauseSwitchFilament; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);

		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ibPauseBack:
				lcd->img_SetWord(handle, ibPauseBack, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibPauseBack);

				THEKERNEL->append_gcode_to_queue("M824", &StreamOutput::NullStream);
				lcd_screens.printing_screen->draw_screen();
				break;
				case ibPauseSwitchFilament:
				lcd->img_SetWord(handle, ibPauseSwitchFilament, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibPauseSwitchFilament);
				lcd_screens.change_filament_screen->draw_screen();
				break;
				default:
				break;
			}
		}
		return 0;
	}

/*
 * LCD4DScreenSaverScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DScreenSaverScreen.h"
#include "LCD4DModule.h"

int LCD4DScreenSaverScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	draw_lcd_images(ibPrepare, ibInformation, handle);

	return 0;
}

int LCD4DScreenSaverScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {

		for (int i = ibSpanish; i <= ibBackLanguage; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);

		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			default:
				break;
		}
	}
	return 0;
}

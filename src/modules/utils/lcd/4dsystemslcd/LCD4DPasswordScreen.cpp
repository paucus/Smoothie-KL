/*
 * LCD4DPasswordScreen.cpp
 *
 *  Created on: Feb 0, 2016
 *      Author: abialet
 */

#include "LCD4DPasswordScreen.h"
#include "LCD4DModule.h"
#include "LCD4DMainScreen.h"
#include "lcd_screens.h"

/* PASSCODE is 4 digits and can not start with zero since its a number...*/
#define PASSCODE 2324

LCD4DPasswordScreen::LCD4DPasswordScreen() : current_passcode(0) {
	this->draw_temperatures_in_current_screen = false;
}

int LCD4DPasswordScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	// Show password prompt
	lcd->img_Disable(handle, ALL);
	draw_lcd_images(ibPassword1, ibPassword0, handle);

	return 0;
}

int LCD4DPasswordScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		draw_lcd_images_touch(ibPassword1, ibPassword0, handle, button);
	} else if (action == TOUCH_RELEASED) {
		draw_lcd_images(ibPassword1, ibPassword0, handle);

		if (button >= ibPassword1 && button <= ibPassword0) {
			current_passcode *= 10;

			switch (button) {
				case ibPassword1: current_passcode += 1; break;
				case ibPassword2: current_passcode += 2; break;
				case ibPassword3: current_passcode += 3; break;
				case ibPassword4: current_passcode += 4; break;
				case ibPassword5: current_passcode += 5; break;
				case ibPassword6: current_passcode += 6; break;
				case ibPassword7: current_passcode += 7; break;
				case ibPassword8: current_passcode += 8; break;
				case ibPassword9: current_passcode += 9; break;
				case ibPassword0: break;
			}

			if (current_passcode == PASSCODE) {
				// Password OK. Settings screen in admin mode
				lcd_screens.settings_screen->draw_screen();
			} else if (current_passcode > 9999) {
				current_passcode = 0;
				lcd_screens.main_screen->draw_screen();
			}
		}
	}
	return 0;
}

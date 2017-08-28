/*
 * LCD4DGenericConfirmationScreen.cpp
 *
 *  Created on: Mar 03, 2016
 *      Author: eai
 */

#include "LCD4DGenericConfirmationScreen.h"
#include "LCD4DModule.h"

#include "LCD4DSelectModelScreen.h"
#include "LCD4DPrintingScreen.h"
#include "lcd_screens.h"

int LCD4DGenericConfirmationScreen::draw_screen() {
	LCD4DScreenWithBackButton::draw_screen();

	lcd->img_Disable(handle, ALL);

	screen_print(10, 10, MEDIUM, TEXT_LEFT, KIKAIBLUE, this->title.c_str());

	screen_print(10, 40, SMALL, TEXT_LEFT, BLACK, this->msg.c_str());

	// NOTE: ibDoCancel ibDoNotCancel might confuse the reader. These would be
	// yes and no respectively. Those names come from the original screen where
	// they were used for the first time.
	draw_lcd_images(ibDoCancel, ibDoNotCancel, handle);

	return 0;
}

int LCD4DGenericConfirmationScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		for (int i = ibDoCancel; i <= ibDoNotCancel; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ibDoCancel:
				this->ok_callback();

				break;
			case ibDoNotCancel:
				this->go_to_previous_screen();

				break;
				default:
				break;
			}
		}
		return 0;
	}

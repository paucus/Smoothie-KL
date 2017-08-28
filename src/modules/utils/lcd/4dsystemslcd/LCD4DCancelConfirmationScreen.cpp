/*
 * LCD4DCancelConfirmationScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DCancelConfirmationScreen.h"
#include "LCD4DModule.h"

#include "LCD4DSelectModelScreen.h"
#include "LCD4DPrintingScreen.h"
#include "lcd_screens.h"

int LCD4DCancelConfirmationScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	screen_print(10, 10, MEDIUM, TEXT_LEFT, KIKAIBLUE, translate(CANCEL_PRINT_HEADER));

	screen_print(10, 40, SMALL, TEXT_LEFT, BLACK, translate(CANCEL_PRINT_TEXT));

	draw_lcd_images(ibDoCancel, ibDoNotCancel, handle);

	return 0;
}

int LCD4DCancelConfirmationScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		for (int i = ibDoCancel; i <= ibDoNotCancel; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ibDoCancel:
				THEKERNEL->append_gcode_to_queue("M826", &StreamOutput::NullStream);
				lcd_screens.select_model_screen->draw_screen();

				break;
			case ibDoNotCancel:
				lcd_screens.printing_screen->draw_screen();

				break;
				default:
				break;
			}
		}
		return 0;
	}

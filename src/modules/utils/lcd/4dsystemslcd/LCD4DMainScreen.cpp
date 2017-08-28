/*
 * LCD4DMainScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DMainScreen.h"
#include "LCD4DModule.h"

#include "LCD4DSelectModelScreen.h"
#include "LCD4DCalibrationScreen.h"
#include "LCD4DInformationScreen.h"
#include "LCD4DPrepareScreen.h"
#include "LCD4DManualControlScreen.h"
#include "lcd_screens.h"
#include "PrinterMode.h"

void LCD4DMainScreen::on_refresh() {
	LCD4DScreen::on_refresh();
}

int LCD4DMainScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	draw_lcd_images(ibPrepare, ibManualControl, handle);

	bool print_enabled = sd_print_allowed();
	THELCD->lcd->img_SetWord(handle, ibPrint, IMAGE_INDEX, print_enabled?0:2);
	THELCD->lcd->img_Show(handle, ibPrint);

	return 0;
}

int LCD4DMainScreen::process_click(int action, int button) {
	static unsigned long timer = millis();
	bool print_enabled = sd_print_allowed();
	if (action == TOUCH_PRESSED) {
		for (int i = ibPrepare; i <= ibManualControl; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == ibPrint && !print_enabled) ? 2 : button == i ? 1 : 0);
			lcd->img_Show(handle, i);
		}
		timer = millis();

	} else if (action == TOUCH_RELEASED) {
		for (int i = ibPrepare; i <= ibManualControl; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == ibPrint && !print_enabled) ? 2 : 0);
			lcd->img_Show(handle, i);
		}
		switch (button) {
			case ibPrint:
				if (print_enabled) {
					lcd_screens.select_model_screen->draw_screen();
				}
				break;
			case ibInformation:
				if ((millis() - timer) >= 10000) {
					lcd_screens.password_screen->draw_screen();
				} else {
					lcd_screens.config_screen->draw_screen();
				}
				break;
			case ibPrepare:
				lcd_screens.prepare_screen->draw_screen();
				break;
			case ibManualControl:
				lcd_screens.manual_control_screen->draw_screen();
				break;
			default:
				break;
		}
	}
	return 0;
}

void LCD4DMainScreen::on_connection_change(connection_event_t* evt) {
	bool print_enabled = sd_print_allowed();
	lcd->img_SetWord(handle, ibPrint, IMAGE_INDEX, !print_enabled ? 2 : 0);
	lcd->img_Show(handle, ibPrint);
}

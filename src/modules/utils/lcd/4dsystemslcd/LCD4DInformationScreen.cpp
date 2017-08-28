/*
 * LCD4DConfigScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DInformationScreen.h"
#include "SerialNumber.h"
#include "LCD4DModule.h"

#include "LCD4DSettingsScreen.h"
#include "LCD4DMainScreen.h"
#include "lcd_screens.h"

int LCD4DInformationScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	lcd->img_Enable(handle, iKikaiLogo);
	lcd->img_Show(handle, iKikaiLogo);

	char temp[100];
	sprintf(temp, translate(SERIAL_NUMBER_FORMAT_LABEL),
			SerialNumber::serial_number.c_str());
	screen_print(0, 120, MEDIUM, TEXT_CENTERED, temp);

	/* Changed so we can correlate installed build with git commits. When ready
	 * need to change for __VERSION_NUMBER__ again.*/
	sprintf(temp, translate(FIRMWARE_VERSION_FORMAT_LABEL), __GITVERSIONSTRING__);

	screen_print(0, 140, MEDIUM, TEXT_CENTERED, temp);

	draw_lcd_images(ibBackInformation,ibSelectLanguage,handle);

	return 0;
}

int LCD4DInformationScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		switch (button) {
			case ibSelectLanguage:
				lcd->img_SetWord(handle, ibSelectLanguage, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibSelectLanguage);
				break;
			case ibBackInformation:
				lcd->img_SetWord(handle, ibBackInformation, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibBackInformation);
				break;
			case ibBack:
				lcd->img_SetWord(handle, ibBack, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibBack);
				break;
			default:
				break;
		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ibSelectLanguage:
//				lcd_screens.language_screen->draw_screen();
				lcd_screens.settings_screen->draw_screen();
				break;
			case ibBackInformation:
				lcd_screens.main_screen->draw_screen();
				break;
			case ibBack:
				lcd_screens.main_screen->draw_screen();
				break;
			default:
				break;
		}

		for (int i = ibBackInformation; i <= ibSelectLanguage; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);
		}

	}
	return 0;
}

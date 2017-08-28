/*
 * LCD4DAutolevelingScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DAutolevelingScreen.h"
#include "LCD4DModule.h"

int LCD4DAutolevelingScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	lcd->img_Enable(handle, iKikaiLogo);
	lcd->img_SetAttributes(handle, iKikaiLogo, I_TOUCH_DISABLE);
	lcd->img_Show(handle, iKikaiLogo);

	screen_print(0, 120, BIG, TEXT_CENTERED, translate(AUTOLEVELING_LABEL));

	screen_print(0, 150, SMALL, TEXT_CENTERED, translate(AUTOLEVELING_TEXT));

	return 0;
}

int LCD4DAutolevelingScreen::process_click(int action, int button) {

	return 0;
}

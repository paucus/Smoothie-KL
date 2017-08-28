/*
 * LCD4DAnalyzingFileScreen.cpp
 *
 *  Created on: Dec 03, 2015
 *      Author: eai
 */

#include "LCD4DAnalyzingFileScreen.h"
#include "LCD4DModule.h"
#include "checksumm.h"
#include "PublicData.h"
#include "Pauser.h"
#include "modules/utils/player/PlayerPublicAccess.h"
#include "modules/tools/temperaturecontrol/TemperatureControlPublicAccess.h"

#include "LCD4DSelectModelScreen.h"

void LCD4DAnalyzingFileScreen::on_refresh() {
	LCD4DScreen::on_refresh();
}

int LCD4DAnalyzingFileScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	screen_print(0, 80, BIG, TEXT_CENTERED, KIKAIBLUE, translate(ANALYZING_FILE_LABEL));

	return 0;
}

int LCD4DAnalyzingFileScreen::process_click(int action, int button) {
	return 0;
}

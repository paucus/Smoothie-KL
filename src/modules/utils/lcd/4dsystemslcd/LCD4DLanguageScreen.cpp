/*
 * LCD4DLanguageScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DLanguageScreen.h"

#include "LCD4DInformationScreen.h"
#include "lcd_screens.h"

int LCD4DLanguageScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	screen_print(5, 5, SMALL, TEXT_LEFT, translate(LANGUAGE_LABEL));

	lcd->img_Enable(handle, ibSpanish);
	lcd->img_ClearAttributes(handle, ibSpanish, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, ibSpanish, IMAGE_INDEX, (SPANISH == lang) ? 2 : 0);
	lcd->img_Show(handle, ibSpanish);

	lcd->img_Enable(handle, ibEnglish);
	lcd->img_ClearAttributes(handle, ibEnglish, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, ibEnglish, IMAGE_INDEX, (ENGLISH == lang) ? 2 : 0);
	lcd->img_Show(handle, ibEnglish);

	lcd->img_Enable(handle, ibPortuguese);
	lcd->img_ClearAttributes(handle, ibPortuguese, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, ibPortuguese, IMAGE_INDEX, (PORTUGUESE == lang) ? 2 : 0);
	lcd->img_Show(handle, ibPortuguese);

	lcd->img_Enable(handle, ibBackLanguage);
	lcd->img_ClearAttributes(handle, ibBackLanguage, I_TOUCH_DISABLE);
	lcd->img_SetWord(handle, ibBackLanguage, IMAGE_INDEX, 0);
	lcd->img_Show(handle, ibBackLanguage);

	return 0;
}

int LCD4DLanguageScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {

		for (int i = ibSpanish; i <= ibBackLanguage; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);

		}
	} else if (action == TOUCH_RELEASED) {
		language_t previous_lang = lang;
		switch (button) {
			case ibEnglish:
				lang = ENGLISH;
				break;
			case ibSpanish:
				lang = SPANISH;
				break;
			case ibPortuguese:
				lang = PORTUGUESE;
				break;
			case ibBackLanguage:
//				lcd_screens.config_screen->draw_screen();
				lcd_screens.settings_screen->draw_screen();
				break;
			default:
				break;
		}

		lcd->img_SetWord(handle, ibSpanish, IMAGE_INDEX, (SPANISH == lang) ? 2 : 0);
		lcd->img_Show(handle, ibSpanish);

		lcd->img_SetWord(handle, ibEnglish, IMAGE_INDEX, (ENGLISH == lang) ? 2 : 0);
		lcd->img_Show(handle, ibEnglish);

		lcd->img_SetWord(handle, ibPortuguese, IMAGE_INDEX, (PORTUGUESE == lang) ? 2 : 0);
		lcd->img_Show(handle, ibPortuguese);

		lcd->img_SetWord(handle, ibBackLanguage, IMAGE_INDEX, 0);
		lcd->img_Show(handle, ibBackLanguage);

		//Need to redraw to reflect language change
		if (lang != previous_lang) {
			draw_screen();
			THEKERNEL->append_gcode_to_queue("M510", &StreamOutput::NullStream);
		}
	}

	return 0;
}

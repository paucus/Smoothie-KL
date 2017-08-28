/*
 * LCD4DChoiceScreen.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: eai
 */

#include "LCD4DChoiceScreen.h"
#include "LCD4DModule.h"
#include "lcd_screens.h"

#define NUMBER_OF_ITEMS_PER_PAGE 3

#define ITEM_BACK_COLOR WHITE
#define ITEM_TOGGLE_COLOR KIKAITOGBLUE
#define NO_IMAGE_SELECTED -1

#define BTN_LEFT_MARGIN 40
#define BTN_TOP_MARGIN 5
#define MAX_SCREEN_X 320

LCD4DChoiceScreen::LCD4DChoiceScreen(unsigned int labels_count, const char* title) : page(0), labels_count(labels_count), title(title), selected_item(NO_IMAGE_SELECTED) {

}

LCD4DChoiceScreen::~LCD4DChoiceScreen() {
}

void LCD4DChoiceScreen::draw_option_item(int item, bool selected) {
	draw_single_button_label((item % NUMBER_OF_ITEMS_PER_PAGE) + ibSettingsOpt1, selected);
}
void LCD4DChoiceScreen::draw_single_button_label(int image, bool selected) {
	if (image == ibSettingsPrevious || image == ibSettingsNext) {
		lcd->img_SetWord(handle, image, IMAGE_INDEX, selected);
		lcd->img_Show(handle, image);
		// we must also redraw the title
		screen_print(0, 15, BIG, TEXT_CENTERED, title);
	} else if (image == ibSettingsOpt1 || image == ibSettingsOpt2 || image == ibSettingsOpt3) {
		// we must also redraw the text over it
		int i = image - ibSettingsOpt1;
		int item = NUMBER_OF_ITEMS_PER_PAGE*page+i;

		lcd->img_SetWord(handle, image, IMAGE_INDEX, is_selectable(item) && selected);
		lcd->img_Show(handle, image);

		screen_print(BTN_LEFT_MARGIN, BTN_TOP_MARGIN+55+39*i, MEDIUM, TEXT_LEFT, (word)(selected?ITEM_TOGGLE_COLOR:ITEM_BACK_COLOR), BLACK, MAX_SCREEN_X - 2*BTN_LEFT_MARGIN, false, get_label(NUMBER_OF_ITEMS_PER_PAGE*page+i));
	}
}
static lcd_image_enum_t clean_beginning_enum(int page, int labels_count) {
	if (NUMBER_OF_ITEMS_PER_PAGE*(page+1) - labels_count == 2) {
		return ibSettingsOpt2;
	} else {
		return ibSettingsOpt3;
	}
}
void LCD4DChoiceScreen::draw_button_labels(int selected_image) {
	// Redraw the title also
	screen_print(0, 15, BIG, TEXT_CENTERED, title);

	for (int i = 0; i < NUMBER_OF_ITEMS_PER_PAGE; i++) {
		int index = NUMBER_OF_ITEMS_PER_PAGE*page+i;
		if (index < labels_count) {
			bool curr_image_is_selected = selected_image==ibSettingsOpt1+i && is_selectable(index);
			screen_print(BTN_LEFT_MARGIN, BTN_TOP_MARGIN+55+39*i, MEDIUM, TEXT_LEFT, (word)(curr_image_is_selected?ITEM_TOGGLE_COLOR:ITEM_BACK_COLOR), BLACK, MAX_SCREEN_X - 2*BTN_LEFT_MARGIN, false,
					get_label(index));
		}
	}
	// clean the background area
	if (NUMBER_OF_ITEMS_PER_PAGE*(page+1) > labels_count) {
		draw_lcd_images(clean_beginning_enum(page, labels_count), ibSettingsOpt3, handle);
	}
}

int LCD4DChoiceScreen::draw_screen(){
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);
	draw_lcd_images(labels_count > NUMBER_OF_ITEMS_PER_PAGE?ibSettingsPrevious:ibSettingsBack, ibSettingsOpt3, handle);

	draw_button_labels(NO_IMAGE_SELECTED);

	return 0;
}


int LCD4DChoiceScreen::process_click(int status, int image){

	if (status == TOUCH_PRESSED) {
		bool something_changed = false;
		switch (image){
		case ibSettingsPrevious:
			lcd->img_SetWord(handle, ibSettingsPrevious, IMAGE_INDEX, 1);
			lcd->img_Show(handle, ibSettingsPrevious);
			break;
		case ibSettingsNext:
			lcd->img_SetWord(handle, ibSettingsNext, IMAGE_INDEX, 1);
			lcd->img_Show(handle, ibSettingsNext);
			break;
		}
		for (int i = ibSettingsPrevious; i <= ibSettingsOpt3; i++) {
			int index = NUMBER_OF_ITEMS_PER_PAGE*page+i - ibSettingsOpt1;
			if (i == image && index < labels_count && is_selectable(index)) {
				something_changed = true;
				selected_item = image;
			}
		}
		if (something_changed) {
			draw_single_button_label(selected_item, true);
		}

	} else if (status == TOUCH_RELEASED) {
		if (selected_item != NO_IMAGE_SELECTED) {
			draw_single_button_label(selected_item, false);
		}
		if (image != ibSettingsPrevious && image != ibSettingsNext) {
			// Don't redraw them as we will have to redraw the whole string anyway
			draw_button_labels(NO_IMAGE_SELECTED);
		}

		switch (image) {
			case ibSettingsPrevious:
				lcd->img_SetWord(handle, ibSettingsPrevious, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibSettingsPrevious);

				if (page > 0) {
					page--;
				}

				// redraw
				draw_button_labels(NO_IMAGE_SELECTED);
				break;
			case ibSettingsNext:
				lcd->img_SetWord(handle, ibSettingsNext, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibSettingsNext);

				if (page < (labels_count -1)/NUMBER_OF_ITEMS_PER_PAGE) {
					page++;
				}

				// redraw
				draw_button_labels(NO_IMAGE_SELECTED);
				break;
			case ibSettingsBack:
				lcd->img_SetWord(handle, ibSettingsBack, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibSettingsBack);

				get_back_screen().draw_screen();
				break;
			case ibSettingsOpt1:
			case ibSettingsOpt2:
			case ibSettingsOpt3:
				if (page*NUMBER_OF_ITEMS_PER_PAGE+(image - ibSettingsOpt1) < labels_count && is_selectable(page*NUMBER_OF_ITEMS_PER_PAGE+(image - ibSettingsOpt1))) {
					return on_choice(page*NUMBER_OF_ITEMS_PER_PAGE+(image - ibSettingsOpt1));
				}

				break;
		}
		for (int i = ibSettingsPrevious; i <= ibSettingsBack; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
			lcd->img_Show(handle, i);
		}
	}
	return 0;
}


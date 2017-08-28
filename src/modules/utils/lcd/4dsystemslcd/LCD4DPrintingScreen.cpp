/*
 * LCD4DPrintingScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DPrintingScreen.h"
#include "LCD4DModule.h"
#include "libs/PublicData.h"
#include "TemperatureControlPublicAccess.h"
#include "PlayerPublicAccess.h"

#include "LCD4DCancelConfirmationScreen.h"
#include "LCD4DPauseScreen.h"
#include "LCD4DAdjustmentsScreen.h"
#include "LCD4DSelectModelScreen.h"
#include "utils.h"
#include "font_width_table.h"
#include "lcd_screens.h"
#include "PrinterMode.h"

#define LABELS_RIGHT_SPACING	120
#define TEXTS_RIGHT_SPACING		210

static const char* get_current_filename();

void LCD4DPrintingScreen::on_refresh() {
	/* We are being called every second... do we need to update that often? */

	send_gcode("M827", commandstream);

	LCD4DScreen::on_refresh();

	if (is_playing_sd_card()){
		//file being printed
		const char* filename = get_current_filename();
		if (filename) {
			// We must limit the number of lines to 2 in order not to overlap with the rest of the screen.
			screen_print(0, 40, MEDIUM, TEXT_CENTERED, WHITE, BLACK, 0, true, TEXT_CENTER_FROM_X, 2, filename);
		}

		//labels
		screen_print(LABELS_RIGHT_SPACING, 85, MEDIUM, TEXT_RIGHT, KIKAIBLUE,  translate(PRINTING_SCREEN_ELAPSED));
		screen_print(LABELS_RIGHT_SPACING, 105, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(PRINTING_SCREEN_REMAINING));
		screen_print(LABELS_RIGHT_SPACING, 125, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(PRINTING_SCREEN_PERCENTAGE));

		//values
		if (commandstream->has_status_info()) {
			parser_status_info_t info = commandstream->get_status_info();
			screen_print(TEXTS_RIGHT_SPACING, 85, MEDIUM, TEXT_LEFT, WHITE, BLACK, HOUR_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, trim_left_cstr(info.elapsed_time));
			screen_print(TEXTS_RIGHT_SPACING, 105, MEDIUM, TEXT_LEFT, WHITE, BLACK, HOUR_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, trim_left_cstr(info.remaining_time));
			char pct[5];
			snprintf(pct, 5, "%d%%", info.percentage_complete);
			screen_print(TEXTS_RIGHT_SPACING, 125, MEDIUM, TEXT_LEFT, WHITE, BLACK, PERCENT_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, pct);
		}else{
			screen_print(TEXTS_RIGHT_SPACING, 85, MEDIUM, TEXT_LEFT, WHITE, BLACK, false, TEXT_CENTER_NONE, "");
			screen_print(TEXTS_RIGHT_SPACING, 105, MEDIUM, TEXT_LEFT, WHITE, BLACK, false, TEXT_CENTER_NONE, "");
			screen_print(TEXTS_RIGHT_SPACING, 125, MEDIUM, TEXT_LEFT, WHITE, BLACK, false, TEXT_CENTER_NONE, "");
		}

	}else{
		//labels
		screen_print(LABELS_RIGHT_SPACING, 85, MEDIUM, TEXT_RIGHT, KIKAIBLUE, translate(PRINTING_SCREEN_ELAPSED));

		//values
		if (commandstream->has_status_info()) {
			parser_status_info_t info = commandstream->get_status_info();
			screen_print(TEXTS_RIGHT_SPACING, 85, MEDIUM, TEXT_LEFT, WHITE, BLACK, HOUR_WIDTH(MEDIUM), false, TEXT_CENTER_NONE, 0, info.elapsed_time);
		}else{
			screen_print(TEXTS_RIGHT_SPACING, 85, MEDIUM, TEXT_LEFT, WHITE, BLACK, false, TEXT_CENTER_NONE, "");
		}

	}

}

static const char* get_current_filename() {
	void* result;
	int res = PublicData::get_value(player_checksum, get_progress_checksum, &result);
	if (res) {
		const char* filename = strrchr(((struct pad_progress*)result)->filename.c_str(), '/');
		if (!filename){
			// no slash
			return ((struct pad_progress*)result)->filename.c_str();
		} else {
			return filename+1;	// next char after last slash
		}
	} else {
		// Show nothing (return null, so that the caller function shows nothing)
		return nullptr;
	}
}

int LCD4DPrintingScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	screen_print(0, 5, BIG, TEXT_CENTERED, KIKAIBLUE, translate(PRINTING_SCREEN_HEADER));

	draw_lcd_images(ibCancel, ibPause, handle, cond_il<int>(pause_and_kill_print_allowed(), {}, {ibCancel, ibPause}));

	return 0;
}

int LCD4DPrintingScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		draw_lcd_images_touch( ibCancel, ibPause, handle, button, cond_il<int>(pause_and_kill_print_allowed(), {}, {ibCancel, ibPause}));

	} else if (action == TOUCH_RELEASED) {
		bool cmds_enabled = pause_and_kill_print_allowed();

		switch (button) {
			case ibCancel:
				if (cmds_enabled) {
					lcd->img_SetWord(handle, ibCancel, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibCancel);

					lcd_screens.cancel_confirmation_screen->draw_screen();
				}
				break;
			case ibPause:
				if (cmds_enabled) {
					lcd->img_SetWord(handle, ibPause, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibPause);

					THEKERNEL->append_gcode_to_queue("M825", commandstream);
					lcd_screens.pause_screen->draw_screen();
				}
				break;
			case ibAdjustments:
				lcd->img_SetWord(handle, ibAdjustments, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibAdjustments);

				lcd_screens.adjustments_screen->draw_screen();
				break;
			default:
				break;
		}
	}
	return 0;
}

void LCD4DPrintingScreen::on_connection_change(connection_event_t* evt) {
	draw_screen();
}


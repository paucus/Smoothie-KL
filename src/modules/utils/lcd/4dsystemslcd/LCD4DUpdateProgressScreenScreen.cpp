/*
 * LCD4DUpdateProgressScreenScreen.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: abialet
 */

#include "LCD4DUpdateProgressScreenScreen.h"
#include "LCD4DModule.h"

#define PROGRESS_BAR_X 60
#define PROGRESS_BAR_Y 220
#define PROGRESS_BAR_WIDTH 200
#define PROGRESS_BAR_HEIGHT 10
#define SHOW_CONSOLE_TEXT 0
#define MAX_CHARS_TO_PRINT 100

int LCD4DUpdateProgressScreenScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	if (!SHOW_CONSOLE_TEXT) {
		lcd->img_Enable(handle, iKikaiLogo);
		lcd->img_Show(handle, iKikaiLogo);
	}

	lcd->txt_FontID(0);
	lcd->txt_BGcolour(WHITE);
	lcd->txt_FGcolour(BLACK);

	return 0;
}

void LCD4DUpdateProgressScreenScreen::printf_above_progress(const char* fmt, ...) {
	char buff[MAX_CHARS_TO_PRINT];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buff, MAX_CHARS_TO_PRINT - 1, fmt, argptr);
	va_end(argptr);

	lcd->gfx_RectangleFilled(PROGRESS_BAR_X, PROGRESS_BAR_Y - 10, 320,
	PROGRESS_BAR_Y + 20, WHITE);
	lcd->gfx_MoveTo(PROGRESS_BAR_X, PROGRESS_BAR_Y - 10);
	lcd->putstr(buff);
}

void LCD4DUpdateProgressScreenScreen::printf(const char* fmt, ...) {
	static int line = -1;

	if (SHOW_CONSOLE_TEXT) {
		char buff[MAX_CHARS_TO_PRINT];
		va_list argptr;
		va_start(argptr, fmt);
		vsnprintf(buff, MAX_CHARS_TO_PRINT - 1, fmt, argptr);
		va_end(argptr);

		if (line < 20) {
			line++;
		} else {
			lcd->gfx_Cls();
			line = 0;
		}

		lcd->txt_MoveCursor(line, 0);
		lcd->putstr(buff);
	}
}

int LCD4DUpdateProgressScreenScreen::process_click(int action, int button) {
	return 0;
}

bool LCD4DUpdateProgressScreenScreen::start_progress_bar() {
	lcd->gfx_RectangleFilled(PROGRESS_BAR_X, PROGRESS_BAR_Y,
	PROGRESS_BAR_WIDTH + PROGRESS_BAR_X,
	PROGRESS_BAR_Y + PROGRESS_BAR_HEIGHT, LIGHTGREY);
	lcd->gfx_RectangleFilled(PROGRESS_BAR_X + 1, PROGRESS_BAR_Y + 1,
	PROGRESS_BAR_WIDTH + PROGRESS_BAR_X - 2,
	PROGRESS_BAR_Y + PROGRESS_BAR_HEIGHT - 2, WHITE);

	return true;
}

bool LCD4DUpdateProgressScreenScreen::stop_progress_bar() {
	lcd->gfx_RectangleFilled(PROGRESS_BAR_X, PROGRESS_BAR_Y,
	PROGRESS_BAR_WIDTH + PROGRESS_BAR_X,
	PROGRESS_BAR_Y + PROGRESS_BAR_HEIGHT, WHITE);

	return true;
}

void LCD4DUpdateProgressScreenScreen::update_progress(double pct_completed) {
	static unsigned int state = 0;

	state++;

	double pct_to_px = min(100.0, pct_completed) * (PROGRESS_BAR_WIDTH - 2);

	lcd->gfx_RectangleFilled(PROGRESS_BAR_X + 1, PROGRESS_BAR_Y + 1,
	PROGRESS_BAR_X + pct_to_px,
	PROGRESS_BAR_Y + PROGRESS_BAR_HEIGHT - 2, KIKAIBLUE);

	lcd->gfx_MoveTo(310, 220);

	char to_print;
	switch (state % 4) {
		case 0:
			to_print = '|';
			break;
		case 1:
			to_print = '/';
			break;
		case 2:
			to_print = '-';
			break;
		case 3:
			to_print = '\\';
			break;
		default:
			break;
	}

	lcd->putCH(to_print);
}

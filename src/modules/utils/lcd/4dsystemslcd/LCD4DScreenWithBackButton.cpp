/*
 * LCD4DScreenWithBackButton.cpp
 *
 *  Created on: Apr 22, 2015
 *      Author: eai
 */

#include "LCD4DScreenWithBackButton.h"
#include "lcd_screens.h"

bool LCD4DScreenWithBackButton::push_next = true;

LCD4DScreenWithBackButton::LCD4DScreenWithBackButton() {

}

LCD4DScreenWithBackButton::~LCD4DScreenWithBackButton() {
}

int LCD4DScreenWithBackButton::draw_screen() {
	//LCD4DScreen::draw_screen(); DO NOT call its parent draw_screen. That would clean the screens stack.
	// Instead, push the current window if it's not the current one (we don't want to re-stack a screen)
	if (push_next){
		if (LCD4DModule::showing_screen != this)
			LCD4DModule::push_screen_shown();
	} else {
		push_next = true;
	}
	/* Need to set ourselves as the showing screen, to get the click event!*/
	LCD4DModule::showing_screen = this;

	on_draw();
	return 0;
}

LCD4DScreen* LCD4DScreenWithBackButton::go_to_previous_screen() {
	return go_to_previous_screen((LCD4DScreen*)lcd_screens.main_screen);
}
LCD4DScreen* LCD4DScreenWithBackButton::go_to_previous_screen(LCD4DScreen* fallback) {
	push_next = false;	// disable pushing screens to the stack temporarily (otherwise it would be re-stacked)
	LCD4DScreen* next_scr = LCD4DModule::pop_screen_and_draw(fallback);
	push_next = true;	// re-enable stacking
	return next_scr;
}


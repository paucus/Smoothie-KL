/*
 * CriticalErrorNotification.cpp
 *
 *  Created on: Apr 30, 2015
 *      Author: eai
 */

#include "CriticalErrorNotification.h"
#include "LCD4DMessageScreen.h"
#include "LCD4DCriticalErrorScreen.h"
#include "lcd_screens.h"

CriticalErrorNotification::CriticalErrorNotification(const char* title, const char* msg) : title(title), msg(msg) {
}

CriticalErrorNotification::~CriticalErrorNotification() {
}

void CriticalErrorNotification::on_module_loaded() {
	this->register_for_event(ON_SECOND_TICK);
	if (lcd_screens.critical_error_screen){
		lcd_screens.critical_error_screen->set_message(title, msg);
		lcd_screens.critical_error_screen->draw_screen();
	}
}

void CriticalErrorNotification::on_second_tick(void*) {
	if (!lcd_screens.critical_error_screen.is_ptr(LCD4DModule::showing_screen)){
		lcd_screens.critical_error_screen->set_message(title, msg);
		lcd_screens.critical_error_screen->draw_screen();
	}
}

CriticalErrorNotification* CriticalErrorNotification::instance_missing_sd_card_error() {
	return new CriticalErrorNotification(translate(MISSING_SD_CARD_HEADER), translate(MISSING_SD_CARD_TEXT));
}
CriticalErrorNotification* CriticalErrorNotification::instance_invalid_dimensions_error() {
	return new CriticalErrorNotification(translate(INVALID_DIMENSIONS_HEADER), translate(INVALID_DIMENSIONS_TEXT));
}

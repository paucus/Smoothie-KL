/*
 * LCD4DCriticalErrorScreen.cpp
 *
 *  Created on: May 04, 2015
 *      Author: eai
 */

#include "LCD4DCriticalErrorScreen.h"
#include "LCD4DModule.h"
#include "USBSerial.h"

// this variable is used to stop the usb serial in order to kill a print.
extern USBSerial usbserial;

int LCD4DCriticalErrorScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	lcd->img_Disable(handle, ALL);

	if (this->title) {
		screen_print(10, 10, MEDIUM, TEXT_LEFT, KIKAIBLUE, this->title);
	}
	if (this->msg) {
		screen_print(10, 40, SMALL, TEXT_LEFT, BLACK, this->msg);
	}

	// This is a critical error. We must keep showing the message permanently and interrupt the USB communication.
	usbserial.set_paused(true);

	return 0;
}

int LCD4DCriticalErrorScreen::process_click(int action, int button) {
	return 0;
}

void LCD4DCriticalErrorScreen::set_message(const char* title, const char* msg){
	this->title = title;
	this->msg = msg;
}


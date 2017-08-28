/*
 * LCD4DGenericConfirmationScreen.h
 *
 *  Created on: Mar 03, 2016
 *      Author: eai
 */

#ifndef LCD4DGENERICCONFIRMATIONSCREEN_H_
#define  LCD4DGENERICCONFIRMATIONSCREEN_H_

#include "LCD4DScreenWithBackButton.h"
#include <functional>

class LCD4DGenericConfirmationScreen: public LCD4DScreenWithBackButton {
	public:
		int draw_screen();
		int process_click(int action, int button);

		void set_ok_callback(std::function<void()> callback) { this->ok_callback = callback;};
		void set_confirmation_message(std::string title, std::string msg) { this->title = title; this->msg = msg;};
	private:
		std::function<void()> ok_callback;
		std::string title;
		std::string msg;
};

#endif /*  LCD4DGENERICCONFIRMATIONSCREEN_H_ */

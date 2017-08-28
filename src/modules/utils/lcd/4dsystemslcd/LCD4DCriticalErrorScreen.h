/*
 * LCD4DCriticalErrorScreen.h
 *
 *  Created on: May 04, 2015
 *      Author: eai
 */

#ifndef LCD4DCRITICALERRORSCREEN_H_
#define  LCD4DCRITICALERRORSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DCriticalErrorScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void set_message(const char* title, const char* msg);
	private:
		const char* title;
		const char* msg;
};

#endif /*  LCD4DCRITICALERRORSCREEN_H_ */

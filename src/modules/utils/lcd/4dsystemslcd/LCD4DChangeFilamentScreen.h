/*
 * LCD4DChangeFilamentScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DCHANGEFILAMENTSCREEN_H_
#define  LCD4DCHANGEFILAMENTSCREEN_H_

#include "LCD4DScreenWithBackButton.h"

class LCD4DChangeFilamentScreen: public LCD4DScreenWithBackButton {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void on_periodic_tick();
		static bool was_called_from_eof_event;
};

#endif /*  LCD4DCHANGEFILAMENTSCREEN_H_ */

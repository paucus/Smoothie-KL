/*
 * LCD4DPauseScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DPAUSESCREEN_H_
#define  LCD4DPAUSESCREEN_H_

#include "LCD4DScreen.h"

class LCD4DPauseScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		//void do_timed_screen_refresh_action();
	private:
};

#endif /*  LCD4DPAUSESCREEN_H_ */

/*
 * LCD4DPrepareScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DPREPARESCREEN_H_
#define LCD4DPREPARESCREEN_H_

#include "LCD4DScreen.h"
#include "LCD4DModule.h"

class LCD4DPrepareScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
	private:
		static bool should_refresh_buttons;
};

#endif /* LCD4DPREPARESCREEN_H_ */

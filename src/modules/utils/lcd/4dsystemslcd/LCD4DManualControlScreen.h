/*
 * LCD4DManualControlScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DMANUALCONTROLSCREEN_H_
#define  LCD4DMANUALCONTROLSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DManualControlScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void on_periodic_tick();
};

#endif /*  LCD4DMANUALCONTROLSCREEN_H_ */

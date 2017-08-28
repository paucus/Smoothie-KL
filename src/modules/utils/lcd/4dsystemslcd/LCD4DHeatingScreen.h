/*
 * LCD4DHeatingScreen.h
 *
 *  Created on: Jun 25, 2015
 *      Author: idlt
 */

#ifndef LCD4DHEATINGSCREEN_H_
#define  LCD4DHEATINGSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DHeatingScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void on_periodic_tick();
	private:
		bool heating_cancelled = false;
};

#endif /*  LCD4DHEATINGSCREEN_H_ */

/*
 * LCD4DPrintDoneScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DPRINTDONESCREEN_H_
#define  LCD4DPRINTDONESCREEN_H_

#include "LCD4DScreen.h"

class LCD4DPrintDoneScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void set_print_time(unsigned int print_secs);
	private:
		char total_print_time_str[sizeof("00:00:00")];
};

#endif /*  LCD4DPRINTDONESCREEN_H_ */

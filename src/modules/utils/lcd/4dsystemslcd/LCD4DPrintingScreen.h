/*
 * LCD4DPrintingScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DPRINTINGSCREEN_H_
#define  LCD4DPRINTINGSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DPrintingScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void on_connection_change(connection_event_t* evt);
};

#endif /*  LCD4DPRINTINGSCREEN_H_ */

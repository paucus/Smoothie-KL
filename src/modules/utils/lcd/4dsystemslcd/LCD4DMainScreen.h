/*
 * LCD4DMainScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DMAINSCREEN_H_
#define LCD4DMAINSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DMainScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void on_connection_change(connection_event_t* evt);
};

#endif /* LCD4DMAINSCREEN_H_ */

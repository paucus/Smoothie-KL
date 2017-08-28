/*
 * LCD4DScreenSaverScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DSCREENSAVERSCREEN_H_
#define  LCD4DSCREENSAVERSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DScreenSaverScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
};

#endif /*  LCD4DSCREENSAVERSCREEN_H_ */

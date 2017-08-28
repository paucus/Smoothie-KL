/*
 * LCD4DAutolevelingScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DAUTOLEVELINGSCREEN_H_
#define  LCD4DAUTOLEVELINGSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DAutolevelingScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
};

#endif /*  LCD4DAUTOLEVELINGSCREEN_H_ */

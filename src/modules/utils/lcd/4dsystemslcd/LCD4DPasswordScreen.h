/*
 * LCD4DPasswordScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DPASSWORDSCREEN_H_
#define  LCD4DPASSWORDSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DPasswordScreen: public LCD4DScreen {
	public:
		LCD4DPasswordScreen();
		virtual ~LCD4DPasswordScreen() {};
		int draw_screen();
		int process_click(int action, int button);

	private:
		int current_passcode;
};

#endif /*  LCD4DPASSWORDSCREEN_H_ */

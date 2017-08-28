/*
 * LCD4DLanguageScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DLANGUAGESCREEN_H_
#define LCD4DLANGUAGESCREEN_H_

#include "LCD4DScreen.h"
#include "LCD4DModule.h"

class LCD4DLanguageScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
};

#endif /* LCD4DLANGUAGESCREEN_H_ */

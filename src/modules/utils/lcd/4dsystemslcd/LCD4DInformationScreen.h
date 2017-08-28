/*
 * LCD4DConfigScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DINFORMATIONSCREEN_H_
#define LCD4DINFORMATIONSCREEN_H_

#include "LCD4DScreen.h"
#include "ImagesConstants.h"

class LCD4DInformationScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
};

#endif /* LCD4DCONFIGSCREEN_H_ */

/*
 * LCD4DCancelConfirmationScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DCANCELCONFIRMATIONSCREEN_H_
#define  LCD4DCANCELCONFIRMATIONSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DCancelConfirmationScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);

	private:
};

#endif /*  LCD4DCANCELCONFIRMATIONSCREEN_H_ */

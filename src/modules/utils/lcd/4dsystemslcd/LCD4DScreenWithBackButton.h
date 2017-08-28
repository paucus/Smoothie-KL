/*
 * LCD4DScreenWithBackButton.h
 *
 *  Created on: Apr 22, 2015
 *      Author: eai
 */

#ifndef LCD4DSCREENWITHBACKBUTTON_H_
#define LCD4DSCREENWITHBACKBUTTON_H_

#include "LCD4DScreen.h"
#include "LCD4DModule.h"

class LCD4DScreenWithBackButton: public LCD4DScreen {
public:
	LCD4DScreenWithBackButton();
	virtual ~LCD4DScreenWithBackButton();

	virtual int draw_screen();
	virtual LCD4DScreen* go_to_previous_screen();
	virtual LCD4DScreen* go_to_previous_screen(LCD4DScreen* fallback);
private:
	static bool push_next;
};

#endif /* LCD4DSCREENWITHBACKBUTTON_H_ */

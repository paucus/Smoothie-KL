/*
 * LCD4DSettingsScreen.h
 *
 *  Created on: Nov 16, 2015
 *      Author: eai
 */

#ifndef LCD4DSETTINGSSCREEN_H_
#define LCD4DSETTINGSSCREEN_H_

#include "LCD4DChoiceScreen.h"

class LCD4DSettingsScreen: public LCD4DChoiceScreen {
public:
	LCD4DSettingsScreen();
	virtual ~LCD4DSettingsScreen();
protected:
	int on_choice(int num);
	const char* get_label(int num);
	LCD4DScreen& get_back_screen();
};

#endif /* LCD4DSETTINGSSCREEN_H_ */

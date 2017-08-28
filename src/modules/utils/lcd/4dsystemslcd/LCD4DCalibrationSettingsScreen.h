/*
 * LCD4DCalibrationSettingsScreen.h
 *
 *  Created on: Jan 04, 2015
 *      Author: eai
 */

#ifndef LCD4DCALIBRATIONSETTINGSSCREEN_H_
#define LCD4DCALIBRATIONSETTINGSSCREEN_H_

#include "LCD4DChoiceScreen.h"

class LCD4DCalibrationSettingsScreen: public LCD4DChoiceScreen {
public:
	LCD4DCalibrationSettingsScreen();
	virtual ~LCD4DCalibrationSettingsScreen();
protected:
	int on_choice(int num);
	const char* get_label(int num);
	LCD4DScreen& get_back_screen();
};

#endif /* LCD4DSETTINGSSCREEN_H_ */

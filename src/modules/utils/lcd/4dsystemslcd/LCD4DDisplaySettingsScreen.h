/*
 * LCD4DDisplaySettingsScreen.h
 *
 *  Created on: Dec 10, 2015
 *      Author: eai
 */

#ifndef LCD4DDISPLAYSETTINGSSCREEN_H_
#define LCD4DDISPLAYSETTINGSSCREEN_H_

#include "LCD4DChoiceScreen.h"

#define LCD4D_DISPLAY_SETT_NUMBER_OF_OPTIONS 1

#define DISPLAY_SETTINGS_BUFF_LEN 40

class LCD4DDisplaySettingsScreen: public LCD4DChoiceScreen {
public:
	LCD4DDisplaySettingsScreen();
	virtual ~LCD4DDisplaySettingsScreen();
protected:
	int on_choice(int num);
	bool is_selectable(int num);
	const char* get_label(int num);
	LCD4DScreen& get_back_screen();
private:
	const char* print_public_data_addr(const char* format_ok, const char* format_fail, uint16_t csb);
	char buff[DISPLAY_SETTINGS_BUFF_LEN];
};

#endif /* LCD4DDISPLAYSETTINGSSCREEN_H_ */

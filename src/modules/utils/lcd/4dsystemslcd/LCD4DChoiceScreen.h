/*
 * LCD4DChoiceScreen.h
 *
 *  Created on: Nov 16, 2015
 *      Author: eai
 */

#ifndef LCD4DCHOICESCREEN_H_
#define LCD4DCHOICESCREEN_H_

#include "LCD4DScreen.h"

class LCD4DChoiceScreen : public LCD4DScreen {
public:
	LCD4DChoiceScreen(unsigned int labels_count, const char* title);
	virtual ~LCD4DChoiceScreen();
	int draw_screen();
	int process_click(int action, int button);
protected:
	void draw_option_item(int item, bool selected);
	virtual int on_choice(int num) = 0;
	virtual bool is_selectable(int num) {return true;};
	virtual const char* get_label(int num) = 0;
	virtual LCD4DScreen& get_back_screen() = 0;
	unsigned int page;
	unsigned int labels_count;
private:
	void draw_button_labels(int selected_image);
	void draw_single_button_label(int selected_image, bool selected);
	const char* title;
	int selected_item;
};

#endif /* LCD4DCHOICESCREEN_H_ */

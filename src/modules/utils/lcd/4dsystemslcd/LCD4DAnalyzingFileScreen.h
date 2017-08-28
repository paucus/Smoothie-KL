/*
 * LCD4DAnalyzingFileScreen.h
 *
 *  Created on: Dec 03, 2015
 *      Author: eai
 */

#ifndef LCD4DANALYZINGFILESCREEN_H_
#define  LCD4DANALYZINGFILESCREEN_H_

#include "LCD4DScreen.h"

class LCD4DAnalyzingFileScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
};

#endif /*  LCD4DANALYZINGFILESCREEN_H_ */

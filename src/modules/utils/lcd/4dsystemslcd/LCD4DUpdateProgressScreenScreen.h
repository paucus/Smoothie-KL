/*
 * LCD4DUpdateProgressScreenScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DUPDATEPROGRESSSCREENSCREEN_H_
#define  LCD4DUPDATEPROGRESSSCREENSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DUpdateProgressScreenScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);

		void printf(const char* fmt, ...);
		void printf_above_progress(const char* fmt, ...);

		/* progress bar functions */
		bool start_progress_bar();
		void update_progress(double pct_completed);
		bool stop_progress_bar();
};

#endif /*  LCD4DUPDATEPROGRESSSCREENSCREEN_H_ */

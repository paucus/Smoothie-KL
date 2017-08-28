/*
 * LCD4DCalibrationScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DCALIBRATIONSCREEN_H_
#define  LCD4DCALIBRATIONSCREEN_H_

#include "LCD4DScreen.h"

class LCD4DCalibrationScreen: public LCD4DScreen {
	public:
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();

	private:
		void go_to_next_step();
		void print_compensation_value();
		void draw_step_screen();
};

#endif /*  LCD4DCALIBRATIONSCREEN_H_ */

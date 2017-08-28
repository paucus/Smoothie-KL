/*
 * LCD4DPrepareCustomTempScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DPREPARECUSTOMTEMPSCREEN_H_
#define  LCD4DPREPARECUSTOMTEMPSCREEN_H_

#include "LCD4DScreenWithBackButton.h"

class LCD4DPrepareCustomTempScreen: public LCD4DScreenWithBackButton {
	public:
		int draw_screen();
		int process_click(int action, int button);

		void set_current_temperature_targets(int hotend_target, int bed_target);
		void set_current_filament_temperatures(int filament, int hotend, int bed);
	private:
		int current_hotend_target_temp;
		int current_bed_target_temp;
		int filament;

		void draw_temperatures();
};

#endif /*  LCD4DPREPARECUSTOMTEMPSCREEN_H_ */

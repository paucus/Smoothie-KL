/*
 * LCD4DMessageScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DMESSAGESCREEN_H_
#define  LCD4DMESSAGESCREEN_H_

#include "LCD4DScreenWithBackButton.h"
#include "alert_event.h"

class LCD4DMessageScreen: public LCD4DScreenWithBackButton {
	public:
		virtual ~LCD4DMessageScreen();
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
		void set_message(const char* message);
		void warn_cold_extrusion();
		void warn_mintemp_triggered(enum mt_alrt_src_e);
		void warn_thermistor_out();
		void warn_unknown_error(const char* msg);
	private:
		char* message;
		static int counter;
};

#endif /*  LCD4DMESSAGESCREEN_H_ */

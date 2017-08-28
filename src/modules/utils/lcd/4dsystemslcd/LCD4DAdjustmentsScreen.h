/*
 * LCD4DAdjustmentsScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DADJUSTMENTSSCREEN_H_
#define  LCD4DADJUSTMENTSSCREEN_H_

#include "LCD4DScreen.h"

typedef enum {
	F_NONE, F_HOTEND_TARGET, F_BED_TARGET
} lcdadj_field_t;

class LCD4DAdjustmentsScreen: public LCD4DScreen {
	public:
		LCD4DAdjustmentsScreen();
		int draw_screen();
		int process_click(int action, int button);

	private:
		int current_speed_modifier;
		int current_flow_rate_modifier;

		lcdadj_field_t field_being_changed;
		float value_begin_changed;

		void print_current_values();
		// This method returns the value of the given field regardless whether it is the value
		// being changed or not.
		float field_value(lcdadj_field_t field);
		// Selects the given field as value being modified. The value while it's being modified
		// is not modified in the original attribute, but instead by using the temporary attribute
		// value_begin_changed. Its value will be transfered once the button is released through
		// the method unset_field_being_changed.
		void set_field_being_changed(lcd_image_enum_t button);
		// Releases the current field as the field being modified and updates its corresponding
		// attribute.
		void unset_field_being_changed();
};

#endif /*  LCD4DADJUSTMENTSSCREEN_H_ */

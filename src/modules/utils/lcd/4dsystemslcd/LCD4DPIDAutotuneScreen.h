/*
 * LCD4DPIDAutotuneScreen.h
 *
 *  Created on: Aug 26, 2015
 *      Author: eai
 */

#ifndef LCD4DPIDAUTOTUNESCREEN_H_
#define LCD4DPIDAUTOTUNESCREEN_H_

#include "LCD4DScreen.h"

class LCD4DPIDAutotuneScreen : public LCD4DScreen {
public:
	LCD4DPIDAutotuneScreen();
	virtual ~LCD4DPIDAutotuneScreen();

	int draw_screen();
	int process_click(int action, int button);
	void reset_wizard(int pool_index, int name_checksum, float temp);
	void on_pid_tested(float peak_temp, float stable_temp, unsigned int time_to_stabilize, unsigned int time_to_reach_temp);
	void on_finish_autotune();
private:
	void cool_down(int pool_index);
	int savePIDValues(int pool_index);
	int restorePIDValues(int pool_index);
	struct{
	short wizard_step;
	short pool_index;
	};
	int name_checksum;
	float temp;	// target temp when wizard_step = begin, stable temp when wizard_step = tested.
	// for wizard_step = tested
	float peak_temp;
	unsigned int time_to_stabilize;
	unsigned int time_to_reach_temp;
//	PIDTestResultParser result_parser;
	float p;
	float i;
	float d;
};

#endif /* LCD4DPIDAUTOTUNESCREEN_H_ */

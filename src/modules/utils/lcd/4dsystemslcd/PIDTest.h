/*
 * PIDTest.h
 *
 *  Created on: Aug 26, 2015
 *      Author: idlt
 */

#ifndef PIDTEST_H_
#define PIDTEST_H_

#include "Module.h"
#include "TemperatureControl.h"
#include "Gcode.h"
#include "StreamOutput.h"

#define PIDTEST_TIME_INTERVAL 35
#define PIDTEST_NUM_MEASURES_PER_SEC 1
#define PIDTEST_NUM_MEASURES ((int)(PIDTEST_NUM_MEASURES_PER_SEC*PIDTEST_TIME_INTERVAL))
// 1 degree difference at most in PIDTEST_TIME_INTERVAL seconds
#define PIDTEST_STABILITY_TOLLERANCE 1

class PIDTest: public Module {
public:
	PIDTest();
	virtual ~PIDTest();

	void on_module_loaded();
	void on_gcode_received(void*);
	void on_idle(void* args);
	uint32_t on_measure_tick(uint32_t );
	void add_measure(float temp);
private:
	float diff_between_max_and_min();

	float target_temp;
	int stage;
	float peak_temp;
	bool testing;
	int pool_index;
	uint16_t name_checksum;
	TemperatureControl* temp_control;
	bool already_attached;
	int measures_done;

	float* measures;
	int meas_index;
	time_t test_start_timestamp;
	time_t stability_start_timestamp;
	StreamOutput* stream;

};

#endif /* PIDTEST_H_ */

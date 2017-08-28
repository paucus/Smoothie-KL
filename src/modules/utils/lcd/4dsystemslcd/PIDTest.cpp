/*
 * PIDTest.cpp
 *
 *  Created on: Aug 26, 2015
 *      Author: idlt
 */

#include "PIDTest.h"
#include "PublicData.h"
#include "Kernel.h"
#include "SlowTicker.h"
#include "Uptime.h"
#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
#include "pid_autotune_event.h"

#define COOLING_STAGE 0
#define HEATING_STAGE 1
#define STABILIZATION_STAGE 2

#define TARGET_TEMP_COOL_DOWN 40

#define pidtest_enable_checksum CHECKSUM("pidtest_enable")

PIDTest::PIDTest() {
	testing = false;
	temp_control = nullptr;
	stage = COOLING_STAGE;
	measures = nullptr;
	peak_temp = 0;
	meas_index = 0;
	target_temp = 0;
	stability_start_timestamp = 0;
	test_start_timestamp = 0;
	stream = nullptr;
	pool_index = 0;
	already_attached = false;
	measures_done = 0;
	name_checksum = 0;
}

PIDTest::~PIDTest() {
}

void PIDTest::on_module_loaded(){
	if (!THEKERNEL->config->value(pidtest_enable_checksum)->by_default(true)->as_bool()) {
		delete this;
		return;
	}

	this->register_for_event(ON_GCODE_RECEIVED);
	this->register_for_event(ON_IDLE);
}
void PIDTest::on_gcode_received(void* argument){
	Gcode* gcode = static_cast<Gcode*>(argument);
	if (gcode->has_m && gcode->m == 803){ // test PID settings
		if (measures != nullptr){
			// mhh, something is wrong.
			return;
		}
		if (testing) {
			// already testing
			gcode->stream->printf("Already testing PID\r\n");
			return;
		}
		test_start_timestamp = uptime();

		pool_index = (int)gcode->get_value('E');
		target_temp = gcode->get_value('S');

		void *returned_data;
		bool ok = PublicData::get_value( temperature_control_checksum, pool_index_checksum, pool_index, &returned_data );
		if (ok) {
			this->temp_control = *static_cast<TemperatureControl **>(returned_data);
		} else {
			gcode->stream->printf("No temperature control with index %d found\r\n", pool_index);
			return;
		}
		name_checksum = temp_control->name_checksum;

		stream = gcode->stream;
		// register for the tick event
		if (!already_attached) {
			THEKERNEL->slow_ticker->attach(PIDTEST_NUM_MEASURES_PER_SEC, this, &PIDTest::on_measure_tick);
			// We can't detach, so, just mark that we are already attached so that we don't attach again
			already_attached = true;
		}
		measures = new float[PIDTEST_NUM_MEASURES];
		measures_done = 0;
		meas_index = 0;
		testing = true;
		peak_temp = 0;

		if (this->temp_control->get_temperature() > TARGET_TEMP_COOL_DOWN) {
			// Not cold enough. First lower the temperature, then heat.
			stage = COOLING_STAGE;
			temp_control->set_desired_temperature(0);
		} else {
			// Cold enough. Start heating and wait until it reaches the temperature
			stage = HEATING_STAGE;
			temp_control->set_desired_temperature(target_temp);
		}
	} else if (gcode->has_m && gcode->m == 804){ // Cancel PID settings test
		testing = false;
		delete[] measures;
		measures = nullptr;

		int difftime = (int)(uptime() - stability_start_timestamp - PIDTEST_TIME_INTERVAL);
		int difftime2 = (int)(stability_start_timestamp - test_start_timestamp);

		pid_autotune_event_t event;
		event.event = pid_autotune_tested;
		event.pool_index = pool_index;
		event.name_checksum = name_checksum;
		event.temp = 0;
		event.peak_temp = peak_temp;
		event.time_to_stabilize = difftime>0?difftime:0;
		event.time_to_reach_temp = difftime2>0?difftime2:0;
		THEKERNEL->call_event(ON_PID_AUTOTUNE_EVENT, &event);
	}
}

float PIDTest::diff_between_max_and_min(){
	float max = -1e30;
	float min = 1e30;
	for (int i = 0; i < PIDTEST_NUM_MEASURES; ++i) {
		min = fmin(min, measures[i]);
		max = fmax(max, measures[i]);
	}
	return max - min;
}

void PIDTest::add_measure(float temp){
	measures[meas_index++] = temp;
	if (meas_index>=PIDTEST_NUM_MEASURES) meas_index = 0;
}

uint32_t PIDTest::on_measure_tick(uint32_t arg) {
	if (testing) {
		if (!measures) {
			// mhh, something is wrong
			return 0;
		}

		float temp = temp_control->get_temperature();

		if (stage == COOLING_STAGE) {
			if (temp > TARGET_TEMP_COOL_DOWN) {
				// Not cold enough. Keep waiting.
				return 0;
			} else {
				// Temperature has been reached. Now proceed to heat the extruder/heatbed.
				stage = HEATING_STAGE;
				temp_control->set_desired_temperature(target_temp);
			}
		}

		// Append last measure. If it overflows, let it go round.
		add_measure(temp);
		peak_temp = fmax(temp, peak_temp);

		if (measures_done < PIDTEST_NUM_MEASURES){
			measures_done++;
			return 0;
		}

		if (stage == HEATING_STAGE) {
			if (temp > target_temp) {
				stage = STABILIZATION_STAGE;
				stability_start_timestamp = uptime();
			}
		} else { // if (stage == STABILIZATION_STAGE) {
			float diff = diff_between_max_and_min();
			if (diff < PIDTEST_STABILITY_TOLLERANCE) {
				// Now considered stable

				testing = false;
				// Defer the rest of the operation to on_idle because we can't work with dynamic memory.
			}
		}
	}
	return 0;
}

void PIDTest::on_idle(void* args){
	// Free resources that can't be released during tick events
	if (stage == STABILIZATION_STAGE && !testing && measures){
		// Substract from the total time the value PIDTEST_TIME_INTERVAL, as during time the temperature was stable.
		int difftime = (int)(uptime() - stability_start_timestamp - PIDTEST_TIME_INTERVAL);
		int difftime2 = (int)(stability_start_timestamp - test_start_timestamp);
		// temperature is stable, we can use any
		float temp;
		if (meas_index == 0)
			temp = measures[PIDTEST_NUM_MEASURES - 1];
		else
			temp = measures[meas_index - 1];
		if (stream){
			stream->printf("peak: %.3f stable: %.3f time (stabilization): %d time (reach temp): %d\r\n", peak_temp, temp, difftime>0?difftime:0, difftime2>0?difftime2:0);
		}
		pid_autotune_event_t event;
		event.event = pid_autotune_tested;
		event.pool_index = pool_index;
		event.name_checksum = name_checksum;
		event.temp = temp;
		event.peak_temp = peak_temp;
		event.time_to_stabilize = difftime>0?difftime:0;
		event.time_to_reach_temp = difftime2>0?difftime2:0;
		THEKERNEL->call_event(ON_PID_AUTOTUNE_EVENT, &event);

		delete[] measures;
		measures = nullptr;
	}
}

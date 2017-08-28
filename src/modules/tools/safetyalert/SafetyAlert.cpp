/*
 * SafetyAlert.cpp
 *
 *  Created on: Mar 4, 2016
 *      Author: eai
 */

#include "SafetyAlert.h"
#include "utils.h"
#include "Kernel.h"
#include "TemperatureControlPublicAccess.h"
#include "TemperatureControl.h"
#include "alert_event.h"

// Falling 3 degrees causes the hotend to throw an alert
#define THRESHOLD_TEMP_ALERT 3.0

SafetyAlert::SafetyAlert() : hotend_is_heating(false), last_hotend_measure(0), get_measure(false) {
}

SafetyAlert::~SafetyAlert() {
}

void SafetyAlert::on_module_loaded() {
	this->register_for_event(ON_SECOND_TICK);
	this->register_for_event(ON_IDLE);
}

void SafetyAlert::on_second_tick(void * args) {
	// We can't use dynamic memory inside an event, so, postpone until next idle.
	this->get_measure = true;
}
void SafetyAlert::on_idle(void * args) {
	if (this->get_measure) {
		this->get_measure = false;
		// Measure the hotend temperature and verify nothing is going wrong.
		struct pad_temperature t;
		t.id = 0;
		t = get_public_data_val<pad_temperature>(temperature_control_checksum, current_temperature_checksum, hotend_checksum, t);
//		struct pad_temperature *t = get_public_data_ptr<pad_temperature>(temperature_control_checksum, current_temperature_checksum, hotend_checksum, nullptr);
		if (!t.id) {
			// could not get temperature control
			// FIXME FAIL!!!!
		} else {
			bool now_is_heating = t.pwm > 0;
			if (hotend_is_heating && now_is_heating) {
				// Still heating
				if (last_hotend_measure - t.current_temperature > THRESHOLD_TEMP_ALERT) {
					// Warning! Probably the thermistor has gone out of the heater
					// block!
					float zero_temp = 0;
					PublicData::set_value(temperature_control_checksum, hotend_checksum, &zero_temp);

					alert_event_t thermistor_out_event;
					thermistor_out_event.reason = alert_thermistor_out;
					THEKERNEL->call_event(ON_ALERT_TRIGGERED, &thermistor_out_event);
				}
			}

			hotend_is_heating = now_is_heating;
			last_hotend_measure = t.current_temperature;
		}
	}
}


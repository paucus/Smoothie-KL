/*
 * pid_autotune_event.h
 *
 *  Created on: Aug 26, 2015
 *      Author: eai
 */

#ifndef PID_AUTOTUNE_EVENT_H_
#define PID_AUTOTUNE_EVENT_H_

#include "TemperatureControl.h"

typedef enum pid_autotune_events {pid_autotune_begin, pid_autotune_abort, pid_autotune_end, pid_autotune_tested} pid_autotune_events;

typedef struct pid_autotune_event_t{
	pid_autotune_events event;
	int pool_index;
	uint16_t name_checksum;

	// In pid_autotune_begin, this is the target temp. In pid_autotune_abort and pid_autotune_end
	// is the current temp. In pid_autotune_tested it is the stable temp.
	float temp;

	float peak_temp;
	unsigned int time_to_stabilize;
	unsigned int time_to_reach_temp;
	// If these are re-enabled, use union to avoid ocupying extra space
	//	TemperatureControl* temp_control;
	//	int ncycles;

} pid_autotune_event_t;

#endif /* PID_AUTOTUNE_EVENT_H_ */

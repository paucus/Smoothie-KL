/*
 * alert_event.h
 *
 *  Created on: Mar 4, 2016
 *      Author: eai
 */

#ifndef ALERT_EVENT_H_
#define ALERT_EVENT_H_

typedef enum alert_event_reasons_e {alert_mintemp, alert_thermistor_out} alert_event_reasons_t;
enum mt_alrt_src_e {MT_ALRT_SRC_HEATBED, MT_ALRT_SRC_HOTEND, MT_ALRT_SRC_UNKNOWN};
typedef struct mintemp_alert_event_s {
	enum mt_alrt_src_e source;
} mintemp_alert_event_t;

typedef struct alert_event_s {
	alert_event_reasons_t reason;
	union {
		mintemp_alert_event_t mintemp_event;
	};
} alert_event_t;


#endif /* ALERT_EVENT_H_ */

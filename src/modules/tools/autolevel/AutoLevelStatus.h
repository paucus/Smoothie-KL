#ifndef AUTOLEVEL_STATUS_H_
#define AUTOLEVEL_STATUS_H_

typedef enum {al_begin, al_end, al_fail} autolevel_event_t;

typedef struct {
	autolevel_event_t event;
} autolevel_status_change_t;

#endif /* AUTOLEVEL_STATUS_H_ */

#ifndef CONNECTION_EVENT
#define CONNECTION_EVENT

// cs_usb = A USB connection was either opened or closed. Only 1 USB connection can be active at a
// time because there's only one plug.
// cs_telnet = A Telnet connection was either opened or closed. Multiple connections are possible.
// cs_restrictions = In this case, there hasn't been a connection opening or closing, but
// permissions were changed, so this event typically concerns parts of the code that listen to
// connections events.
typedef enum connection_source {cs_usb, cs_telnet, cs_restrictions} connection_source_t;
typedef enum connection_event_type {cet_open, cet_close} connection_event_type_t;
typedef enum restriction_event_type {ret_enabled_restrictions, ret_disabled_restrictions} restriction_event_type_t;
typedef struct connection_event{
	connection_source_t src;
	union {
		connection_event_type_t type;
		restriction_event_type_t rest_action;
	};
} connection_event_t;

#endif // CONNECTION_EVENT

#include "Uptime.h"

#include "SlowTicker.h"
#include "PublicDataRequest.h"
#include "utils.h"

// Global variable that holds the value
volatile time_t uptime_val;
volatile time_t uptime_millis_val;

time_t uptime() {
    return uptime_val;
}

time_t uptime_millis() {
    return uptime_millis_val;
}

void UptimeImpl::on_module_loaded() {
    uptime_val = 0;

    THEKERNEL->slow_ticker->attach( 1.0 , this, &UptimeImpl::tick_second );
    THEKERNEL->slow_ticker->attach( 1000.0 , this, &UptimeImpl::tick_millis );

    // BEGIN MODIF FREE MEM
    // No one accesses uptime using public data.
//    this->register_for_event(ON_GET_PUBLIC_DATA);
//    this->register_for_event(ON_SET_PUBLIC_DATA);
    // END MODIF FREE MEM
    this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
}

uint32_t UptimeImpl::tick_second(uint32_t dummy) {
    uptime_val++;
    return 0;
}

uint32_t UptimeImpl::tick_millis(uint32_t dummy) {
    uptime_millis_val++;    // just for test. should increase one by one really
    return 0;
}

// BEGIN MODIF FREE MEM
//void UptimeImpl::on_get_public_data(void* argument) {
//    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);
//    if(pdr->starts_with(uptime_checksum)) {
//        *static_cast<time_t*>(pdr->get_data_ptr()) = uptime_val;
//        pdr->set_taken();
//    }
//}
//
//void UptimeImpl::on_set_public_data(void* argument) {
//    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);
//    if(pdr->starts_with(uptime_checksum)) {
//        // set requested uptime value
//        time_t t = *static_cast<time_t *>(pdr->get_data_ptr());
//        uptime_val = t;
//        pdr->set_taken();
//    }
//
//}
// END MODIF FREE MEM

void UptimeImpl::on_console_line_received(void * args) {
	SerialMessage* msg = static_cast<SerialMessage *>(args);
	if (msg->message == "help") {
		msg->stream->printf("uptime - prints the time the system has been up\r\n");
	} else if (msg->message == "uptime") {
	    time_t up = uptime();
	    long int hs;
	    int mins, secs;
	    convert_to_time_units(up, &hs, &mins, &secs);
	    char t_str[sizeof("00:00:00")];
	    format_time(t_str, hs, mins, secs);
	    msg->stream->printf("Uptime: %s\n", t_str);
	}
}



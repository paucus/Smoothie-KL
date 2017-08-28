#ifndef UPTIME
#define UPTIME

#include <time.h>

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"
#include "libs/StreamOutput.h"
#include "checksumm.h"

// Returns the time elapsed since the printer booted in seconds.
// As long as a print doesn't take more than 136 years, will be fine :-)
time_t uptime();
time_t uptime_millis();

// BEGIN MODIF FREE MEM
// No one accesses uptime using public data.
//#define uptime_checksum         CHECKSUM("uptime")
// END MODIF FREE MEM

class UptimeImpl : public Module {
public:
    void on_module_loaded();
    uint32_t tick_second(uint32_t dummy);
    uint32_t tick_millis(uint32_t dummy);
    // BEGIN MODIF FREE MEM
//    void on_get_public_data(void* argument);
//    void on_set_public_data(void* argument);
    // END MODIF FREE MEM
    void on_console_line_received(void * args);
};

#endif // UPTIME

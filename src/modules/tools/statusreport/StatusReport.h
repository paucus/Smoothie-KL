#ifndef STATUS_REPORT_H
#define STATUS_REPORT_H

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "libs/Pin.h"
#include "libs/StreamOutput.h"
#include "checksumm.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

// if you modify this list, remember to update the FORMATTED_PRINT_TYPES in the .cpp file
typedef enum {
    FMTPNT_STRING,
    FMTPNT_BOOL,
    FMTPNT_NOT_BOOL,
    FMTPNT_SWITCH_STATE,
    FMTPNT_NOT_SWITCH_STATE,
    FMTPNT_INT,
    FMTPNT_UINT,
    FMTPNT_LONG,
    FMTPNT_ULONG,
    FMTPNT_FLOAT,
    FMTPNT_UNKNOWN
} formatted_print_t;


typedef struct {
    string key;
    formatted_print_t type;
    string label;
} status_report_t;


class StatusReport : public Module
{
    public:
        StatusReport();
        virtual ~StatusReport();

        void on_module_loaded();
        void on_gcode_received(void* argument);
        void on_config_reload(void* argument);
    private:
        vector<status_report_t> values_to_report;
};

#endif

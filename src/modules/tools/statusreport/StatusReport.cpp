#include "StatusReport.h"
#include "libs/Kernel.h"
#include "Config.h"
#include "PublicData.h"
#include "checksumm.h"
#include "utils/Gcode.h"
#include "libs/utils.h"
#include "ConfigValue.h"
#include "modules/tools/switch/SwitchPublicAccess.h"
//#include "Timer.h"

#define statusreport_checksum           CHECKSUM("statusreport")
#define enable_checksum                 CHECKSUM("enable")
#define key_checksum                    CHECKSUM("key")
#define label_checksum                  CHECKSUM("label")
#define type_checksum                   CHECKSUM("type")
// For types, see FormattedPrint.cpp. So far they are string, int, float, bool and not_bool (bool, but with the inverted value)

// if you modify this list, remember to update the formatted_print_t in the .h file
static int FORMATTED_PRINT_TYPES[] = {
    CHECKSUM("string"),
    CHECKSUM("bool"),
    CHECKSUM("not_bool"),
    CHECKSUM("switch_state"),
    CHECKSUM("not_switch_state"),
    CHECKSUM("int"),
    CHECKSUM("uint"),
    CHECKSUM("long"),
    CHECKSUM("ulong"),
    CHECKSUM("float")
};

StatusReport::StatusReport()
{
}

StatusReport::~StatusReport()
{
}

void StatusReport::on_module_loaded()
{
    // Do not do anything if not enabled
    if ( THEKERNEL->config->value( statusreport_checksum, enable_checksum )->by_default(false)->as_bool() == false ) {
        return;
    }

    //register_for_event(ON_CONFIG_RELOAD);
    register_for_event(ON_GCODE_RECEIVED);
    
    // Settings
    this->on_config_reload(this);
}

static formatted_print_t from_string(const char* fmt) {
    int fmt_checksum = get_checksum(fmt);
    for (unsigned int i = 0; i < sizeof(FORMATTED_PRINT_TYPES)/sizeof(int); i++) {
        if (fmt_checksum == FORMATTED_PRINT_TYPES[i]) {
            return (formatted_print_t)i;
        }
    }
    return FMTPNT_UNKNOWN;
}

void StatusReport::on_config_reload(void* argument)
{
    // enumerate values to report
    vector<uint16_t> modules;
    // this method searches for statusreport_checksum.xxxx.enable=true
    THEKERNEL->config->get_module_list( &modules, statusreport_checksum );

    values_to_report.clear();
    for (uint16_t m : modules) {
        if (m != enable_checksum) {
            if (THEKERNEL->config->value( statusreport_checksum, m, enable_checksum )->by_default(true)->as_bool() == true ) {
                status_report_t value_to_report;
                value_to_report.key = THEKERNEL->config->value( statusreport_checksum, m, key_checksum )->required()->as_string();
                value_to_report.label = THEKERNEL->config->value( statusreport_checksum, m, label_checksum )->required()->as_string();
                value_to_report.type = from_string( THEKERNEL->config->value( statusreport_checksum, m, type_checksum )->by_default("string")->as_string().c_str() );
                values_to_report.push_back(value_to_report);
            }
        }
    }
}

static void print_data(void* data, formatted_print_t type, StreamOutput* stream) {
    switch (type) {
        case FMTPNT_STRING:
            stream->printf("%s", (const char*)data);
            break;
        case FMTPNT_BOOL:
            stream->printf((*(const bool*)data)?"1":"0");
            break;
        case FMTPNT_NOT_BOOL:
            stream->printf((*(const bool*)data)?"0":"1");       // the opposite of  BoolFormattedPrint
            break;
        case FMTPNT_SWITCH_STATE:
            stream->printf(((const struct pad_switch*)data)->state?"1":"0");
            break;
        case FMTPNT_NOT_SWITCH_STATE:
            stream->printf(((const struct pad_switch*)data)->state?"0":"1");
            break;
        case FMTPNT_INT:
            stream->printf("%d", *(int*)data);
            break;
        case FMTPNT_UINT:
            stream->printf("%u", *(unsigned int*)data);
            break;
        case FMTPNT_LONG:
            stream->printf("%ld", *(long*)data);
            break;
        case FMTPNT_ULONG:
            stream->printf("%lu", *(unsigned long*)data);
            break;
        case FMTPNT_FLOAT:
            stream->printf("%f", *(float*)data);
            break;
        default: //FMTPNT_UNKNOWN
            stream->printf("?");
    }
}

void StatusReport::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);
    if( gcode->has_m){
        int code = gcode->m;
        if( code == 701 ){       // M701
            // TODO agrega que filtre por pattern
            //string args = get_arguments(gcode->get_command());

            for (status_report_t s : values_to_report) {
                uint16_t req_check_sums[3] = {0,0,0};
                get_checksums(req_check_sums, s.key);
                
                void * data;
                if (PublicData::get_value(req_check_sums[0], req_check_sums[1], req_check_sums[2], &data)) {
                    gcode->stream->printf("%s: ", s.label.c_str());
                    print_data(data, s.type, gcode->stream);
                    gcode->stream->printf("\n");
                }
            }
        }
    }
}


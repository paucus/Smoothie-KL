#ifndef __TEMPERATURECONTROLPUBLICACCESS_H
#define __TEMPERATURECONTROLPUBLICACCESS_H

#include "checksumm.h"

#include <string>

// addresses used for public data access
#define temperature_control_checksum      CHECKSUM("temperature_control")
#define hotend_checksum                   CHECKSUM("hotend")
#define bed_checksum                      CHECKSUM("bed")
#define current_temperature_checksum      CHECKSUM("current_temperature")
#define target_temperature_checksum       CHECKSUM("target_temperature")
#define temperature_pwm_checksum          CHECKSUM("temperature_pwm")
#define pool_index_checksum               CHECKSUM("pool_index")
// BEGIN MODIF max_temp
#define max_temp_checksum                 CHECKSUM("max_temp")
#define MAX_HOTEND_TEMP                   285
#define MAX_BED_TEMP                      120
// END MODIF max_temp
// BEGIN MODIF unblock_temp
#define waiting_checksum                  CHECKSUM("waiting")
// END MODIF unblock_temp
// BEGIN MODIF max_set_temp
#define max_set_temp_checksum              CHECKSUM("max_set_temp")
// END MODIF max_set_temp
#define poll_controls_checksum            CHECKSUM("poll_controllers")

struct pad_temperature {
    float current_temperature;
    float target_temperature;
    int pwm;
    uint16_t id;
    std::string designator;
};
#endif

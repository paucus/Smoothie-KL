#include "FirmwareVersion.h"

#include "libs/utils.h"

#include "Config.h"
#include "ConfigValue.h"
#include "SerialMessage.h"
#include "checksumm.h"
#include "PublicData.h"
#include "PublicDataRequest.h"

#define firm_version_module_enabled_checksum     CHECKSUM("firm_version_module_enabled")

FirmwareVersion::FirmwareVersion(){
}
void FirmwareVersion::on_module_loaded(){
    if (THEKERNEL->config->value( firm_version_module_enabled_checksum )->by_default( true )->as_bool())

    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_GET_PUBLIC_DATA);

    on_config_reload(this);
}
void FirmwareVersion::on_config_reload(void *argument){
}

void FirmwareVersion::on_gcode_received(void *argument){
    Gcode *gcode = static_cast<Gcode *>(argument);

    if (gcode->has_m) {
        if (gcode->m == 115) {
            // Add EXTRUDER_COUNT, PROTOCOL_VERSION
            gcode->stream->printf("FIRMWARE_VERSION:%s.%s GIT_COMMIT:%s FIRMWARE_NAME:Smoothie MACHINE_TYPE:Kikai\r\nok\r\n", __VERSION_NUMBER__, __TIMESTAMP_BUILD__, __GITVERSIONSTRING__);
        }
    }
}

#define MAX_BUILD_DATE_STR_LEN (sizeof(__TIMESTAMP_BUILD__)+1)
#define MAX_GIT_VERSION_STR_LEN (sizeof(__GITVERSIONSTRING__)+1)
void FirmwareVersion::on_get_public_data(void* argument) {
    PublicDataRequest* pdr = static_cast<PublicDataRequest*>(argument);

    if(!pdr->starts_with(firmware_version_checksum)) return;

    if(pdr->second_element_is(number_checksum)) {
        static char v_num[MAX_BUILD_DATE_STR_LEN];
        strcpy(v_num, __TIMESTAMP_BUILD__);
        pdr->set_data_ptr(v_num);
        pdr->set_taken();
    } else if(pdr->second_element_is(commit_checksum)) {
        static char v_num[MAX_GIT_VERSION_STR_LEN];
        strcpy(v_num, __GITVERSIONSTRING__);
        pdr->set_data_ptr(v_num);
        pdr->set_taken();
    }
}


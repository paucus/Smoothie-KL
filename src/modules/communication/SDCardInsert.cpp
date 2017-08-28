#include <string>
using std::string;
#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"
#include "SDCardInsert.h"
#include "libs/StreamOutput.h"
#include "libs/FileStream.h"
#include "SDFAT.h"
#include "Config.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "PublicData.h"
#include "modules/utils/player/PlayerPublicAccess.h"

#if SPLIT_CONFIG_AND_PUBLIC_SD
#include "DualDisk.h"
#include "SDCard.h"

    extern SDFAT* mounter_sd2;
    extern DualDisk sd2;
#endif

SDCardInsert::SDCardInsert() {}

void SDCardInsert::on_module_loaded()
{
    this->register_for_event(ON_GCODE_RECEIVED);
}

static bool is_playing_file_from_sd_card() {
    void *returned_data;
    bool ok = PublicData::get_value( player_checksum, is_playing_checksum, &returned_data );
    if (ok) {
        return *static_cast<bool *>(returned_data);
    } else {
        // Say that the printer is printing, so that it doesn't do anything.
        return true;
    }
}

// When a command is received, if it is a Gcode, dispatch it as an object via an event
void SDCardInsert::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);

    if (gcode->has_m) {
        if (gcode->m == 21) {
            #if SPLIT_CONFIG_AND_PUBLIC_SD
                // M21: Initialize SD card
                // The SD card is initialized. If an SD card is loaded when the machine is switched on, this will happen by default. SD card must be initialized for the other SD functions to work.
                if (!sd_card_already_initialized()) {
                    if (initialize_sd_card()) {
                        gcode->stream->printf("SD card successfully initialized.\r\n");
                    } else {
                        gcode->stream->printf("Failed to initialize SD card.\r\n");
                    }
                } else {
                    gcode->stream->printf("SD card already mounted. Ignoring command.\r\n");
                }
            # else
                gcode->stream->printf("Operation not supported.\r\n");
            #endif
        } else if (gcode->m == 22) {
            #if SPLIT_CONFIG_AND_PUBLIC_SD
                // M22: Release SD card
                // SD card is released and can be physically removed.
                if (sd_card_already_initialized()) {
                    if (release_sd_card()) {
                        gcode->stream->printf("SD card successfully released.\r\n");
                    } else {
                        gcode->stream->printf("Failed to release SD card.\r\n");
                    }
                } else {
                    gcode->stream->printf("File SD card \r\n");
                }
            # else
                gcode->stream->printf("Operation not supported.\r\n");
            #endif
        } else if (gcode->m == 721) {
            #if SPLIT_CONFIG_AND_PUBLIC_SD
                // M721: If not playing from SD card, remount it. If the SD card was not already initialized, try to mount it.
                if (!is_playing_file_from_sd_card()) {
                    if (release_sd_card()) {
                        if (initialize_sd_card()) {
                            gcode->stream->printf("SD card successfully reset.\r\n");
                        } else {
                            gcode->stream->printf("Failed to reinitialize SD card.\r\n");
                        }
                    } else {
                        gcode->stream->printf("Failed to release SD card before reinitializing it.\r\n");
                    }
                } else {
                    gcode->stream->printf("Printing in progress. Remount omitted.\r\n");
                }
            # else
                gcode->stream->printf("Operation not supported.\r\n");
            #endif
        }
    }
}

#if SPLIT_CONFIG_AND_PUBLIC_SD
bool SDCardInsert::sd_card_already_initialized(){
    return mounter_sd2 != nullptr;
}
bool SDCardInsert::initialize_sd_card(){
    // initialize disk (to ensure the card was properly inserted, for example)
    bool sd2ok= (sd2.disk_initialize() == 0);
    if (sd2ok) {
        // if everything is ok, mount it
        mounter_sd2 = new SDFAT(PUBLIC_SD_MOUNT_DIR, &sd2);
    }
    return sd2ok;
}
bool SDCardInsert::release_sd_card(){
    sd2.disk_sync();
    // now unmount the sd card
    delete mounter_sd2;
    mounter_sd2 = nullptr;
    
    return true;
}
#endif



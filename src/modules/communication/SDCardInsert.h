#ifndef SDCARD_INSERT_H
#define SDCARD_INSERT_H

#include <string>
using std::string;
#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"

#include "libs/StreamOutput.h"

class SDCardInsert : public Module {
    public:
        SDCardInsert();

        virtual void on_module_loaded();
        virtual void on_gcode_received(void *argument);
    private:
        #if SPLIT_CONFIG_AND_PUBLIC_SD
        bool sd_card_already_initialized();
        bool initialize_sd_card();
        bool release_sd_card();
        #endif
};

#endif

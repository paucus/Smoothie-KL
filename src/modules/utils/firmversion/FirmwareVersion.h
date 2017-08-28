#ifndef FIRMWARE_VERSION_MODULE
#define FIRMWARE_VERSION_MODULE

#include "FirmwareVersionPublicAccess.h"

#include <string>
using std::string;
#include "libs/Module.h"
#include "libs/Kernel.h"
#include "utils/Gcode.h"

#include "libs/StreamOutput.h"

class FirmwareVersion : public Module {
public:
    FirmwareVersion();
    virtual void on_module_loaded();
    virtual void on_config_reload(void *argument);
    virtual void on_gcode_received(void *argument);
    virtual void on_get_public_data(void* argument);

private:
};

#endif // FIRMWARE_VERSION_MODULE

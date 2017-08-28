/*
 * ShowPrint.h
 *
 *  Created on: Feb 24, 2015
 *      Author: eai
 */

#ifndef SHOWPRINT_H_
#define SHOWPRINT_H_

#include "Module.h"

//
// This module is executed once the print has finished. In that moment, it will lift the extruder
// lift_z millimeters, ensuring that at least it's rised up to go_at_least_to_z position, displacing
// at z_speed (in mm per minute).
//
class ShowPrint: public Module {
public:
    ShowPrint();
    virtual ~ShowPrint();

    virtual void on_module_loaded();
    virtual void on_print_status_change(void* status);
    virtual void on_gcode_received(void* status);
protected:
    float go_at_least_to_z;
    float lift_z;
    float z_speed;
    bool enable_show_print;
};

#endif /* SHOWPRINT_H_ */


/*
 * FilamentTemps.cpp
 *
 *  Created on: Jul 15, 2015
 *      Author: idlt
 */

#include "FilamentTemps.h"
#include "Gcode.h"
#include "StreamOutputPool.h"

float filament_temperatures[4][2] = {
		{210, 65},	// PLA
		{250, 105},	// ABS
		{255, 100},	// NYLON
		{260, 65}	// FLEXIBLE
};

FilamentTemps::FilamentTemps() {
}

FilamentTemps::~FilamentTemps() {
}

void FilamentTemps::on_module_loaded() {
	this->register_for_event(ON_GCODE_RECEIVED);
}

void FilamentTemps::on_gcode_received(void* argument){
	Gcode* gcode = static_cast<Gcode*>(argument);
	int f = 0;
	float b = 0.0;
	float e = 0.0;
	if (gcode->has_m && gcode->m == 801) {	// M801 F<filament> B<bed_temp> E<ext_temp>
		if (gcode->has_letter('F')) {
			f = (int)gcode->get_value('F');
			if (f>=0 && f<=3){
				if (gcode->has_letter('B')){
					b = gcode->get_value('B');
					filament_temperatures[f][1] = b;
				}
				if (gcode->has_letter('E')){
					e = gcode->get_value('E');
					filament_temperatures[f][0] = e;
				}
			}
		}
	} else if (gcode->has_m && (gcode->m == 510 || gcode->m == 503)){
		for (int f = PLA; f < FLEXIBLE+1; f++){
			int fil = f - PLA;
			gcode->stream->printf("M801 F%d B%.3f E%.3f\n", fil, filament_temperatures[fil][1], filament_temperatures[fil][0]);
		}
	}
}

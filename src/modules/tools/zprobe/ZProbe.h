/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ZPROBE_H_
#define ZPROBE_H_

#include "Module.h"
#include "Pin.h"

#include <vector>
// BEGIN MODIF correct_nozzle_distance_to_bed
#include "FunctionInterpolation.h"
// END MODIF correct_nozzle_distance_to_bed

// defined here as they are used in multiple files
#define zprobe_checksum            CHECKSUM("zprobe")
#define leveling_strategy_checksum CHECKSUM("leveling-strategy")
// BEGIN MODIF correct_nozzle_distance_to_bed
#define delta_mm_from_bed_checksum CHECKSUM("delta_mm_from_bed")
// END MODIF correct_nozzle_distance_to_bed

class StepperMotor;
class Gcode;
class StreamOutput;
class LevelingStrategy;

class ZProbe: public Module
{

public:
    void on_module_loaded();
    void on_config_reload(void *argument);
    void on_gcode_received(void *argument);
    void acceleration_tick(void);

    bool wait_for_probe(int& steps);
    bool run_probe(int& steps, bool fast= false);
    bool run_probe_feed(int& steps, float feedrate);
    bool return_probe(int steps);
    bool doProbeAt(int &steps, float x, float y);
    float probeDistance(float x, float y);

    void coordinated_move(float x, float y, float z, float feedrate, bool relative=false);
    void home();

    bool getProbeStatus() { return this->pin.get(); }
    float getSlowFeedrate() const { return slow_feedrate; }
    float getFastFeedrate() const { return fast_feedrate; }
    float getProbeHeight() const { return probe_height; }
    float getMaxZ() const { return max_z; }
    float zsteps_to_mm(float steps);
    // BEGIN MODIF correct_nozzle_distance_to_bed
    int delta_steps_from_bed();
    float delta_mm_from_bed();
    void on_get_public_data(void *argument);
    // END MODIF correct_nozzle_distance_to_bed

private:
    void accelerate(int c);

    volatile float current_feedrate;
    float slow_feedrate;
    float fast_feedrate;
    float return_feedrate;
    float probe_height;
    float max_z;
    volatile struct {
        volatile bool running:1;
        bool is_delta:1;
    };

    Pin pin;
    uint8_t debounce_count;
    std::vector<LevelingStrategy*> strategies;
    // BEGIN MODIF correct_nozzle_distance_to_bed
    FunctionInterpolation temp_correction;
    bool autolevel_on_home;
    // END MODIF correct_nozzle_distance_to_bed
};

#endif /* ZPROBE_H_ */

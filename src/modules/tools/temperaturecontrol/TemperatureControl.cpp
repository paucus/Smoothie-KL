/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

// TODO : THIS FILE IS LAME, MUST BE MADE MUCH BETTER

#include "libs/Module.h"
#include "libs/Kernel.h"
#include <math.h>
#include "TemperatureControl.h"
#include "TemperatureControlPool.h"
#include "libs/Pin.h"
#include "modules/robot/Conveyor.h"
#include "PublicDataRequest.h"

#include "PublicData.h"
#include "ToolManagerPublicAccess.h"
#include "StreamOutputPool.h"
#include "Config.h"
#include "checksumm.h"
#include "Gcode.h"
#include "SlowTicker.h"
#include "Pauser.h"
#include "ConfigValue.h"
#include "PID_Autotuner.h"
#include "SerialMessage.h"
#include "utils.h"

// Temp sensor implementations:
#include "Thermistor.h"
#include "max31855.h"

#include "MRI_Hooks.h"
// BEGIN MODIF mintemp
#include "alert_event.h"
// END MODIF mintemp

#define UNDEFINED -1

#define sensor_checksum                    CHECKSUM("sensor")

#define readings_per_second_checksum       CHECKSUM("readings_per_second")
#define max_pwm_checksum                   CHECKSUM("max_pwm")
#define pwm_frequency_checksum             CHECKSUM("pwm_frequency")
#define bang_bang_checksum                 CHECKSUM("bang_bang")
#define hysteresis_checksum                CHECKSUM("hysteresis")
#define heater_pin_checksum                CHECKSUM("heater_pin")
#define max_temp_checksum                  CHECKSUM("max_temp")
#define min_temp_checksum                  CHECKSUM("min_temp")

#define get_m_code_checksum                CHECKSUM("get_m_code")
#define set_m_code_checksum                CHECKSUM("set_m_code")
#define set_and_wait_m_code_checksum       CHECKSUM("set_and_wait_m_code")

#define designator_checksum                CHECKSUM("designator")

#define p_factor_checksum                  CHECKSUM("p_factor")
#define i_factor_checksum                  CHECKSUM("i_factor")
#define d_factor_checksum                  CHECKSUM("d_factor")

#define i_max_checksum                     CHECKSUM("i_max")
#define windup_checksum                    CHECKSUM("windup")

#define preset1_checksum                   CHECKSUM("preset1")
#define preset2_checksum                   CHECKSUM("preset2")

TemperatureControl::TemperatureControl(uint16_t name, int index)
{
    name_checksum= name;
    pool_index= index;
    waiting= false;
    temp_violated= false;
    sensor= nullptr;
    readonly= false;
}

TemperatureControl::~TemperatureControl()
{
    delete sensor;
}

void TemperatureControl::on_module_loaded()
{

    // We start not desiring any temp
    this->target_temperature = UNDEFINED;
    this->sensor_settings= false; // set to true if sensor settings have been overriden

    // Settings
    this->load_config();

    // Register for events
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_GET_PUBLIC_DATA);

    if(!this->readonly) {
        this->register_for_event(ON_SECOND_TICK);
        this->register_for_event(ON_MAIN_LOOP);
        this->register_for_event(ON_SET_PUBLIC_DATA);
        this->register_for_event(ON_HALT);
    }
}

void TemperatureControl::on_halt(void *arg)
{
    if(arg == nullptr) {
        // turn off heater
        this->o = 0;
        this->heater_pin.set(0);
        this->target_temperature = UNDEFINED;
    }
}

void TemperatureControl::shutdown_heaters() {
    this->o = 0;
    this->heater_pin.set(0);
    this->target_temperature = UNDEFINED;
}

void TemperatureControl::on_main_loop(void *argument)
{
    if (this->temp_violated) {
        this->temp_violated = false;
        THEKERNEL->streams->printf("Error: MINTEMP or MAXTEMP triggered. Check your temperature sensors!\n");
        // BEGIN MODIF mintemp
        // Throw a mintemp event
//         THEKERNEL->streams->printf("HALT asserted - reset or M999 required\n");
//         THEKERNEL->call_event(ON_HALT, nullptr);
        shutdown_heaters();
        alert_event_t mintemp_event;
        mintemp_event.reason = alert_mintemp;
        mintemp_event.mintemp_event.source = (this->name_checksum == bed_checksum)?MT_ALRT_SRC_HEATBED:(this->name_checksum == hotend_checksum)?MT_ALRT_SRC_HOTEND:MT_ALRT_SRC_UNKNOWN;
        THEKERNEL->call_event(ON_ALERT_TRIGGERED, &mintemp_event);
        // END MODIF mintemp
    }
}

// Get configuration from the config file
void TemperatureControl::load_config()
{

    // General config
    this->set_m_code          = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, set_m_code_checksum)->by_default(104)->as_number();
    this->set_and_wait_m_code = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, set_and_wait_m_code_checksum)->by_default(109)->as_number();
    this->get_m_code          = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, get_m_code_checksum)->by_default(105)->as_number();
    this->readings_per_second = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, readings_per_second_checksum)->by_default(20)->as_number();

    this->designator          = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, designator_checksum)->by_default(string("T"))->as_string();

    // Max and min temperatures we are not allowed to get over (Safety)
    this->max_temp = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, max_temp_checksum)->by_default(300)->as_number();
    this->min_temp = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, min_temp_checksum)->by_default(0)->as_number();

    // Heater pin
    this->heater_pin.from_string( THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, heater_pin_checksum)->by_default("nc")->as_string());
    if(this->heater_pin.connected()){
        this->readonly= false;
        this->heater_pin.as_output();

    } else {
        this->readonly= true;
    }

    // For backward compatibility, default to a thermistor sensor.
    std::string sensor_type = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, sensor_checksum)->by_default("thermistor")->as_string();

    // Instantiate correct sensor (TBD: TempSensor factory?)
    delete sensor;
    sensor = nullptr; // In case we fail to create a new sensor.
    if(sensor_type.compare("thermistor") == 0) {
        sensor = new Thermistor();
    } else if(sensor_type.compare("max31855") == 0) {
        sensor = new Max31855();
    } else {
        sensor = new TempSensor(); // A dummy implementation
    }
    sensor->UpdateConfig(temperature_control_checksum, this->name_checksum);

    this->preset1 = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, preset1_checksum)->by_default(0)->as_number();
    this->preset2 = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, preset2_checksum)->by_default(0)->as_number();


    // sigma-delta output modulation
    this->o = 0;

    if(!this->readonly) {
        // used to enable bang bang control of heater
        this->use_bangbang = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, bang_bang_checksum)->by_default(false)->as_bool();
        this->hysteresis = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, hysteresis_checksum)->by_default(2)->as_number();
        this->windup = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, windup_checksum)->by_default(false)->as_bool();
        this->heater_pin.max_pwm( THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, max_pwm_checksum)->by_default(255)->as_number() );
        this->heater_pin.set(0);
        set_low_on_debug(heater_pin.port_number, heater_pin.pin);
        // activate SD-DAC timer
        THEKERNEL->slow_ticker->attach( THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, pwm_frequency_checksum)->by_default(2000)->as_number(), &heater_pin, &Pwm::on_tick);
    }


    // reading tick
    THEKERNEL->slow_ticker->attach( this->readings_per_second, this, &TemperatureControl::thermistor_read_tick );
    this->PIDdt = 1.0 / this->readings_per_second;

    // PID
    setPIDp( THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, p_factor_checksum)->by_default(10 )->as_number() );
    setPIDi( THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, i_factor_checksum)->by_default(0.3f)->as_number() );
    setPIDd( THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, d_factor_checksum)->by_default(200)->as_number() );

    if(!this->readonly) {
        // set to the same as max_pwm by default
        this->i_max = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, i_max_checksum   )->by_default(this->heater_pin.max_pwm())->as_number();
    }

    this->iTerm = 0.0;
    this->lastInput = -1.0;
    this->last_reading = 0.0;
    // BEGIN MODIF max_set_temp
    // 285 degrees Celsius is enough to handle any extruder up to ABS and gives a margin of safety in case a PTFE tube is used.
    this->max_set_temp = THEKERNEL->config->value(temperature_control_checksum, this->name_checksum, max_set_temp_checksum)->by_default((this->name_checksum == bed_checksum)?MAX_BED_TEMP:MAX_HOTEND_TEMP)->as_number();
    // END MODIF max_set_temp
}

void TemperatureControl::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode *>(argument);
    if (gcode->has_m) {

        if( gcode->m == this->get_m_code ) {
            char buf[32]; // should be big enough for any status
            int n = snprintf(buf, sizeof(buf), "%s:%3.1f /%3.1f @%d ", this->designator.c_str(), this->get_temperature(), ((target_temperature <= 0) ? 0.0 : target_temperature), this->o);
            gcode->txt_after_ok.append(buf, n);
            return;
        }

        if (gcode->m == 305) { // set or get sensor settings
            if (gcode->has_letter('S') && (gcode->get_value('S') == this->pool_index)) {
                TempSensor::sensor_options_t args= gcode->get_args();
                args.erase('S'); // don't include the S
                if(args.size() > 0) {
                    // set the new options
                    if(sensor->set_optional(args)) {
                        this->sensor_settings= true;
                    }else{
                        gcode->stream->printf("Unable to properly set sensor settings, make sure you specify all required values\n");
                    }
                }else{
                    // don't override
                    this->sensor_settings= false;
                }

            }else if(!gcode->has_letter('S')) {
                gcode->stream->printf("%s(S%d): using %s\n", this->designator.c_str(), this->pool_index, this->readonly?"Readonly" : this->use_bangbang?"Bangbang":"PID");
                sensor->get_raw();
                TempSensor::sensor_options_t options;
                if(sensor->get_optional(options)) {
                    for(auto &i : options) {
                        // foreach optional value
                        gcode->stream->printf("%s(S%d): %c %1.18f\n", this->designator.c_str(), this->pool_index, i.first, i.second);
                    }
                }
            }

            return;
        }

        // readonly sensors don't handle the rest
        if(this->readonly) return;

        if (gcode->m == 143) {
            if (gcode->has_letter('S') && (gcode->get_value('S') == this->pool_index)) {
                if(gcode->has_letter('P')) {
                    max_temp= gcode->get_value('P');

                } else {
                    gcode->stream->printf("Nothing set NOTE Usage is M143 S0 P300 where <S> is the hotend index and <P> is the maximum temp to set\n");
                }

            }else if(gcode->get_num_args() == 0) {
                gcode->stream->printf("Maximum temperature for %s(%d) is %f°C\n", this->designator.c_str(), this->pool_index, max_temp);
            }

        } else if (gcode->m == 301) {
            if (gcode->has_letter('S') && (gcode->get_value('S') == this->pool_index)) {
                if (gcode->has_letter('P'))
                    setPIDp( gcode->get_value('P') );
                if (gcode->has_letter('I'))
                    setPIDi( gcode->get_value('I') );
                if (gcode->has_letter('D'))
                    setPIDd( gcode->get_value('D') );
                if (gcode->has_letter('X'))
                    this->i_max = gcode->get_value('X');
                if (gcode->has_letter('Y'))
                    this->heater_pin.max_pwm(gcode->get_value('Y'));

            }else if(!gcode->has_letter('S')) {
                gcode->stream->printf("%s(S%d): Pf:%g If:%g Df:%g X(I_max):%g max pwm: %d O:%d\n", this->designator.c_str(), this->pool_index, this->p_factor, this->i_factor / this->PIDdt, this->d_factor * this->PIDdt, this->i_max, this->heater_pin.max_pwm(), o);
            }

        } else if (gcode->m == 500 || gcode->m == 503) { // M500 saves some volatile settings to config override file, M503 just prints the settings
            gcode->stream->printf(";PID settings:\nM301 S%d P%1.4f I%1.4f D%1.4f X%1.4f Y%d\n", this->pool_index, this->p_factor, this->i_factor / this->PIDdt, this->d_factor * this->PIDdt, this->i_max, this->heater_pin.max_pwm());

            gcode->stream->printf(";Max temperature setting:\nM143 S%d P%1.4f\n", this->pool_index, this->max_temp);

            if(this->sensor_settings) {
                // get or save any sensor specific optional values
                TempSensor::sensor_options_t options;
                if(sensor->get_optional(options) && !options.empty()) {
                    gcode->stream->printf(";Optional temp sensor specific settings:\nM305 S%d", this->pool_index);
                    for(auto &i : options) {
                        gcode->stream->printf(" %c%1.18f", i.first, i.second);
                    }
                    gcode->stream->printf("\n");
                }
            }

        } else if( ( gcode->m == this->set_m_code || gcode->m == this->set_and_wait_m_code ) && gcode->has_letter('S')) {
            // this only gets handled if it is not controlled by the tool manager or is active in the toolmanager
            this->active = true;

            // this is safe as old configs as well as single extruder configs the toolmanager will not be running so will return false
            // this will also ignore anything that the tool manager is not controlling and return false, otherwise it returns the active tool
            void *returned_data;
            bool ok = PublicData::get_value( tool_manager_checksum, is_active_tool_checksum, this->name_checksum, &returned_data );
            if (ok) {
                uint16_t active_tool_name =  *static_cast<uint16_t *>(returned_data);
                this->active = (active_tool_name == this->name_checksum);
            }

            if(this->active) {
                // required so temp change happens in order
                THEKERNEL->conveyor->wait_for_empty_queue();

                float v = gcode->get_value('S');

                if (v == 0.0) {
                    this->target_temperature = UNDEFINED;
                    this->heater_pin.set((this->o = 0));
                } else {
                    // BEGIN MODIF max_set_temp
                    if (v > this->max_set_temp){
                        // truncate temp
                        gcode->stream->printf("Requested temperature %0.1f exceeds max temperature %0.1f.", v, this->max_set_temp);
                        v = this->max_set_temp;
                    }
                    // END MODIF max_set_temp
                    this->set_desired_temperature(v);
                    // wait for temp to be reached, no more gcodes will be fetched until this is complete
                    if( gcode->m == this->set_and_wait_m_code) {
                        if(isinf(get_temperature()) && isinf(sensor->get_temperature())) {
                            // BEGIN MODIF mintemp
                            THEKERNEL->streams->printf("Error: MINTEMP or MAXTEMP triggered. Check your temperature sensors!\n");
                            // Throw a mintemp event
//                            THEKERNEL->streams->printf("Temperature reading is unreliable HALT asserted - reset or M999 required\n");
//                            THEKERNEL->call_event(ON_HALT, nullptr);
                            shutdown_heaters();
                            alert_event_t mintemp_event;
                            mintemp_event.reason = alert_mintemp;
                            THEKERNEL->call_event(ON_ALERT_TRIGGERED, &mintemp_event);
                            return;
                        }

                        this->waiting = true; // on_second_tick will announce temps
                        // BEGIN MODIF unblock_temp
                        while ( get_temperature() < target_temperature && this->waiting) {
                        // END MODIF unblock_temp
                            THEKERNEL->call_event(ON_IDLE, this);
                            // check if ON_HALT was called (usually by kill button)
                            if(THEKERNEL->is_halted() || this->target_temperature == UNDEFINED) {
                                THEKERNEL->streams->printf("Wait on temperature aborted by kill\n");
                                break;
                            }
                        }
                        this->waiting = false;
                    }
                }
            }
        }
    }
}

void TemperatureControl::on_get_public_data(void *argument)
{
    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);

    if(!pdr->starts_with(temperature_control_checksum)) return;

    if(pdr->second_element_is(pool_index_checksum)) {
        // asking for our instance pointer if we have this pool_index
        if(pdr->third_element_is(this->pool_index)) {
            static void *return_data;
            return_data = this;
            pdr->set_data_ptr(&return_data);
            pdr->set_taken();
        }

    }else if(pdr->second_element_is(poll_controls_checksum)) {
        // polling for all temperature controls
        // add our data to the list which is passed in via the data_ptr

        std::vector<struct pad_temperature> *v= static_cast<std::vector<pad_temperature>*>(pdr->get_data_ptr());

        struct pad_temperature t;
        // setup data
        t.current_temperature = this->get_temperature();
        t.target_temperature = (target_temperature <= 0) ? 0 : this->target_temperature;
        t.pwm = this->o;
        t.designator= this->designator;
        t.id= this->name_checksum;
        v->push_back(t);
        pdr->set_taken();

    }else if(pdr->second_element_is(current_temperature_checksum)) {
        // if targeted at us
        if(pdr->third_element_is(this->name_checksum)) {
            // ok this is targeted at us, so set the requ3sted data in the pointer passed into us
            struct pad_temperature *t= static_cast<pad_temperature*>(pdr->get_data_ptr());
            t->current_temperature = this->get_temperature();
            t->target_temperature = (target_temperature <= 0) ? 0 : this->target_temperature;
            t->pwm = this->o;
            t->designator= this->designator;
            t->id= this->name_checksum;
            pdr->set_taken();
        }
    // BEGIN MODIF max_set_temp
    }else if(pdr->second_element_is(max_set_temp_checksum)) {
        // if targeted at us
        if(pdr->third_element_is(this->name_checksum)) {
            *(static_cast<float*>(pdr->get_data_ptr())) = this->max_set_temp;
            pdr->set_taken();
        }
    // END MODIF max_set_temp
    // BEGIN MODIF unblock_temp
    }else if(pdr->second_element_is(waiting_checksum)) {
        // if targeted at us
        if(pdr->third_element_is(this->name_checksum)) {
            *(static_cast<bool*>(pdr->get_data_ptr())) = this->waiting;
            pdr->set_taken();
        }
    // END MODIF unblock_temp
    }
}

void TemperatureControl::on_set_public_data(void *argument)
{
    PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);

    if(!pdr->starts_with(temperature_control_checksum)) return;

    // BEGIN MODIF unblock_temp
    if(pdr->second_element_is(waiting_checksum) && pdr->third_element_is(this->name_checksum)){
        this->waiting = *static_cast<bool *>(pdr->get_data_ptr());;
        pdr->set_taken();
        return;
    }
    // END MODIF unblock_temp

    if(!pdr->second_element_is(this->name_checksum)) return;

    // ok this is targeted at us, so set the temp
    // NOTE unlike the M code this will set the temp now not when the queue is empty
    float t = *static_cast<float *>(pdr->get_data_ptr());
    this->set_desired_temperature(t);
    pdr->set_taken();
}

void TemperatureControl::set_desired_temperature(float desired_temperature)
{
    // Never go over the configured max temperature
    // BEGIN MODIF max_set_temp
    if( desired_temperature > this->max_set_temp ){
        desired_temperature = this->max_set_temp;
    }
    // END MODIF max_set_temp

    // BEGIN MODIF fix
    // This can cause more problems than advantages
//    if (desired_temperature == 1.0F)
//        desired_temperature = preset1;
//    else if (desired_temperature == 2.0F)
//        desired_temperature = preset2;
    // END MODIF fix

    float last_target_temperature= target_temperature;
    target_temperature = desired_temperature;
    if (desired_temperature <= 0.0F){
        // turning it off
        heater_pin.set((this->o = 0));

    }else if(last_target_temperature <= 0.0F) {
        // if it was off and we are now turning it on we need to initialize
        this->lastInput= last_reading;
        // set to whatever the output currently is See http://brettbeauregard.com/blog/2011/04/improving-the-beginner%E2%80%99s-pid-initialization/
        this->iTerm= this->o;
        if (this->iTerm > this->i_max) this->iTerm = this->i_max;
        else if (this->iTerm < 0.0) this->iTerm = 0.0;
    }
}

float TemperatureControl::get_temperature()
{
    return last_reading;
}

uint32_t TemperatureControl::thermistor_read_tick(uint32_t dummy)
{
    float temperature = sensor->get_temperature();
    if(!this->readonly && target_temperature > 2) {
        if (isinf(temperature) || temperature < min_temp || temperature > max_temp) {
            this->temp_violated = true;
            target_temperature = UNDEFINED;
            heater_pin.set((this->o = 0));
        } else {
            pid_process(temperature);
        }
    }

    last_reading = temperature;
    return 0;
}

/**
 * Based on https://github.com/br3ttb/Arduino-PID-Library
 */
void TemperatureControl::pid_process(float temperature)
{
    if(use_bangbang) {
        // bang bang is very simple, if temp is < target - hysteresis turn on full else if  temp is > target + hysteresis turn heater off
        // good for relays
        if(temperature > (target_temperature + hysteresis) && this->o > 0) {
            heater_pin.set(false);
            this->o = 0; // for display purposes only

        } else if(temperature < (target_temperature - hysteresis) && this->o <= 0) {
            if(heater_pin.max_pwm() >= 255) {
                // turn on full
                this->heater_pin.set(true);
                this->o = 255; // for display purposes only
            } else {
                // only to whatever max pwm is configured
                this->heater_pin.pwm(heater_pin.max_pwm());
                this->o = heater_pin.max_pwm(); // for display purposes only
            }
        }
        return;
    }

    // regular PID control
    float error = target_temperature - temperature;

    float new_I = this->iTerm + (error * this->i_factor);
    if (new_I > this->i_max) new_I = this->i_max;
    else if (new_I < 0.0) new_I = 0.0;
    if(!this->windup) this->iTerm= new_I;

    float d = (temperature - this->lastInput);

    // calculate the PID output
    // TODO does this need to be scaled by max_pwm/256? I think not as p_factor already does that
    this->o = (this->p_factor * error) + new_I - (this->d_factor * d);

    if (this->o >= heater_pin.max_pwm())
        this->o = heater_pin.max_pwm();
    else if (this->o < 0)
        this->o = 0;
    else if(this->windup)
        this->iTerm = new_I; // Only update I term when output is not saturated.

    this->heater_pin.pwm(this->o);
    this->lastInput = temperature;
}

void TemperatureControl::on_second_tick(void *argument)
{
    if (waiting)
        THEKERNEL->streams->printf("%s:%3.1f /%3.1f @%d\n", designator.c_str(), get_temperature(), ((target_temperature <= 0) ? 0.0 : target_temperature), o);
}

void TemperatureControl::setPIDp(float p)
{
    this->p_factor = p;
}

void TemperatureControl::setPIDi(float i)
{
    this->i_factor = i * this->PIDdt;
}

void TemperatureControl::setPIDd(float d)
{
    this->d_factor = d / this->PIDdt;
}
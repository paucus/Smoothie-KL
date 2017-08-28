#include "ConfigValue.h"
#include "StreamOutputPool.h"

#include "libs/Kernel.h"
#include "libs/utils.h"
#include "libs/Pin.h"
#include "Pwm.h"


#define printErrorandExit(...) THEKERNEL->streams->printf(__VA_ARGS__); // exit(1);

#include <vector>
#include <stdio.h>

// BEGIN MODIF cache_value_proxy
HeavyWeightConfigValueProxy HeavyWeightConfigValueProxy::instance;
HeavyWeightConfigValueProxy* HeavyWeightConfigValueProxy::do_conf(ConfigValue* p){
    instance.clear();
	instance.p = p;
    if (p) {
		instance.value = p->value;
		instance.found = true;
    }
    return &instance;
}
HeavyWeightConfigValueProxy::HeavyWeightConfigValueProxy() {
    clear();
}
void HeavyWeightConfigValueProxy::clear() {
    this->p = NULL;
    this->value = "";
    this->default_int = 0;
    this->default_double = 0.0F;
    this->found = false;
    this->default_set = false;
}
HeavyWeightConfigValueProxy* HeavyWeightConfigValueProxy::required(){
    if( !this->found ) {
        printErrorandExit("could not find config setting, please see http://smoothieware.org/configuring-smoothie\r\n");
    }
    return this;
}
float HeavyWeightConfigValueProxy::as_number(){
    if( this->found == false && this->default_set == true ) {
        return this->default_double;
    } else {
        char *endptr = NULL;
        string str = remove_non_number(this->value);
        const char *cp= str.c_str();
        float result = strtof(cp, &endptr);
        if( endptr <= cp ) {
            printErrorandExit("config setting with value '%s' and checksums[%04X,%04X,%04X] is not a valid number, please see http://smoothieware.org/configuring-smoothie\r\n", this->value.c_str(), p?p->check_sums[0]:0, p?p->check_sums[1]:0, p?p->check_sums[2]:0 );
        }
        return result;
    }
}
int HeavyWeightConfigValueProxy::as_int(){
    if( this->found == false && this->default_set == true ) {
        return this->default_int;
    } else {
        char *endptr = NULL;
        string str = remove_non_number(this->value);
        const char *cp= str.c_str();
        int result = strtol(cp, &endptr, 10);
        if( endptr <= cp ) {
            printErrorandExit("config setting with value '%s' and checksums[%04X,%04X,%04X] is not a valid int, please see http://smoothieware.org/configuring-smoothie\r\n", this->value.c_str(), p?p->check_sums[0]:0, p?p->check_sums[1]:0, p?p->check_sums[2]:0 );
        }
        return result;
    }
}
bool HeavyWeightConfigValueProxy::as_bool(){
    if( this->found == false && this->default_set == true ) {
        return this->default_int;
    } else {
        return this->value.find_first_of("ty1") != string::npos;
    }
}
string HeavyWeightConfigValueProxy::as_string(){
    return this->value;
}
HeavyWeightConfigValueProxy* HeavyWeightConfigValueProxy::by_default(float val){
    this->default_set = true;
    this->default_double = val;
    return this;
}
HeavyWeightConfigValueProxy* HeavyWeightConfigValueProxy::by_default(string val){
    if( this->found ) {
        return this;
    }
    this->default_set = true;
    this->value = val;
    return this;
}
HeavyWeightConfigValueProxy* HeavyWeightConfigValueProxy::by_default(int val){
    this->default_set = true;
    this->default_int = val;
    this->default_double = val; // we need to set both becuase sometimes an integer is passed when it should be a float
    return this;
}
bool HeavyWeightConfigValueProxy::has_characters( const char *mask )
{
    if( this->value.find_first_of(mask) != string::npos ) {
        return true;
    } else {
        return false;
    }
}
bool HeavyWeightConfigValueProxy::is_inverted(){
    return this->has_characters("!");
}
// END MODIF cache_value_proxy

ConfigValue::ConfigValue()
{
    clear();
}

void ConfigValue:: clear()
{
    // BEGIN MODIF cache_value_proxy
//    this->found = false;
//    this->default_set = false;
    // END MODIF cache_value_proxy
    this->check_sums[0] = 0x0000;
    this->check_sums[1] = 0x0000;
    this->check_sums[2] = 0x0000;
    // BEGIN MODIF cache_value_proxy
//    this->default_double= 0.0F;
//    this->default_int= 0;
    // END MODIF cache_value_proxy
    this->value= "";
}

ConfigValue::ConfigValue(uint16_t *cs) {
    memcpy(this->check_sums, cs, sizeof(this->check_sums));
    // BEGIN MODIF cache_value_proxy
//    this->found = false;
//    this->default_set = false;
    // END MODIF cache_value_proxy
    this->value= "";
}

ConfigValue::ConfigValue(const ConfigValue& to_copy)
{
    // BEGIN MODIF cache_value_proxy
//    this->found = to_copy.found;
//    this->default_set = to_copy.default_set;
    // END MODIF cache_value_proxy
    memcpy(this->check_sums, to_copy.check_sums, sizeof(this->check_sums));
    this->value.assign(to_copy.value);
}

ConfigValue& ConfigValue::operator= (const ConfigValue& to_copy)
{
    if( this != &to_copy ){
        // BEGIN MODIF cache_value_proxy
//        this->found = to_copy.found;
//        this->default_set = to_copy.default_set;
        // END MODIF cache_value_proxy
        memcpy(this->check_sums, to_copy.check_sums, sizeof(this->check_sums));
        this->value.assign(to_copy.value);
    }
    return *this;
}

// BEGIN MODIF cache_value_proxy
HeavyWeightConfigValueProxy *ConfigValue::required()
// END MODIF cache_value_proxy
{
    // BEGIN MODIF cache_value_proxy
//    if( !this->found ) {
//        printErrorandExit("could not find config setting, please see http://smoothieware.org/configuring-smoothie\r\n");
//    }
//    return this;
    return HeavyWeightConfigValueProxy::do_conf(this)->required();
    // END MODIF cache_value_proxy
}

float ConfigValue::as_number()
{
    // BEGIN MODIF cache_value_proxy
//    if( this->found == false && this->default_set == true ) {
//        return this->default_double;
//    } else {
//        char *endptr = NULL;
//        string str = remove_non_number(this->value);
//        const char *cp= str.c_str();
//        float result = strtof(cp, &endptr);
//        if( endptr <= cp ) {
//            printErrorandExit("config setting with value '%s' and checksums[%04X,%04X,%04X] is not a valid number, please see http://smoothieware.org/configuring-smoothie\r\n", this->value.c_str(), this->check_sums[0], this->check_sums[1], this->check_sums[2] );
//        }
//        return result;
//    }
    return HeavyWeightConfigValueProxy::do_conf(this)->as_number();
    // END MODIF cache_value_proxy
}

int ConfigValue::as_int()
{
    // BEGIN MODIF cache_value_proxy
//    if( this->found == false && this->default_set == true ) {
//        return this->default_int;
//    } else {
//        char *endptr = NULL;
//        string str = remove_non_number(this->value);
//        const char *cp= str.c_str();
//        int result = strtol(cp, &endptr, 10);
//        if( endptr <= cp ) {
//            printErrorandExit("config setting with value '%s' and checksums[%04X,%04X,%04X] is not a valid int, please see http://smoothieware.org/configuring-smoothie\r\n", this->value.c_str(), this->check_sums[0], this->check_sums[1], this->check_sums[2] );
//        }
//        return result;
//    }
    return HeavyWeightConfigValueProxy::do_conf(this)->as_int();
    // END MODIF cache_value_proxy
}

std::string ConfigValue::as_string()
{
    return this->value;
}

bool ConfigValue::as_bool()
{
    // BEGIN MODIF cache_value_proxy
//    if( this->found == false && this->default_set == true ) {
//        return this->default_int;
//    } else {
//        return this->value.find_first_of("ty1") != string::npos;
//    }
    return HeavyWeightConfigValueProxy::do_conf(this)->as_bool();
    // END MODIF cache_value_proxy
}

// BEGIN MODIF cache_value_proxy
HeavyWeightConfigValueProxy *ConfigValue::by_default(int val)
// END MODIF cache_value_proxy
{
    // BEGIN MODIF cache_value_proxy
//    this->default_set = true;
//    this->default_int = val;
//    this->default_double = val; // we need to set both becuase sometimes an integer is passed when it should be a float
//    return this;
    return HeavyWeightConfigValueProxy::do_conf(this)->by_default(val);
    // END MODIF cache_value_proxy
}

// BEGIN MODIF cache_value_proxy
HeavyWeightConfigValueProxy *ConfigValue::by_default(float val)
// END MODIF cache_value_proxy
{
    // BEGIN MODIF cache_value_proxy
//    this->default_set = true;
//    this->default_double = val;
//    return this;
    return HeavyWeightConfigValueProxy::do_conf(this)->by_default(val);
    // END MODIF cache_value_proxy
}

// BEGIN MODIF cache_value_proxy
HeavyWeightConfigValueProxy *ConfigValue::by_default(string val)
// END MODIF cache_value_proxy
{
    // BEGIN MODIF cache_value_proxy
//    if( this->found ) {
//        return this;
//    }
//    this->default_set = true;
//    this->value = val;
//    return this;
    return HeavyWeightConfigValueProxy::do_conf(this)->by_default(val);
    // END MODIF cache_value_proxy
}

// BEGIN MODIF cache_value_proxy
//bool ConfigValue::has_characters( const char *mask )
//{
//    if( this->value.find_first_of(mask) != string::npos ) {
//        return true;
//    } else {
//        return false;
//    }
//}
// END MODIF cache_value_proxy

bool ConfigValue::is_inverted()
{
// BEGIN MODIF cache_value_proxy
//    return this->has_characters("!");
    return HeavyWeightConfigValueProxy::do_conf(this)->is_inverted();
// END MODIF cache_value_proxy
}


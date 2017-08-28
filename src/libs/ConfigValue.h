/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIGVALUE_H
#define CONFIGVALUE_H

#include <string>
using std::string;

// BEGIN MODIF cache_value_proxy
class ConfigValue;

// This class holds temporarily default values so as not to need to keep them in memory all the time.
// Previously all these values were held in memory for every ConfigValue at module loading, which
// required 10 extra bytes per object.
class HeavyWeightConfigValueProxy {
public:
    static HeavyWeightConfigValueProxy* do_conf(ConfigValue* p);
    HeavyWeightConfigValueProxy();
    HeavyWeightConfigValueProxy* required();
    float as_number();
    int as_int();
    bool as_bool();
    string as_string();

    HeavyWeightConfigValueProxy* by_default(float val);
    HeavyWeightConfigValueProxy* by_default(string val);
    HeavyWeightConfigValueProxy* by_default(int val);
    bool has_characters( const char *mask );
    bool is_inverted();
    void clear();
private:
    static HeavyWeightConfigValueProxy instance;
    string value;
    ConfigValue* p;
    int default_int;
    float default_double;
    bool found;
    bool default_set;
};
// END MODIF cache_value_proxy

class ConfigValue{
    public:
        ConfigValue();
        ConfigValue(uint16_t *check_sums);
        ConfigValue(const ConfigValue& to_copy);
        ConfigValue& operator= (const ConfigValue& to_copy);
        void clear();
        // BEGIN MODIF cache_value_proxy
        HeavyWeightConfigValueProxy* required();
        // END MODIF cache_value_proxy
        float as_number();
        int as_int();
        bool as_bool();
        string as_string();

        // BEGIN MODIF cache_value_proxy
        HeavyWeightConfigValueProxy* by_default(float val);
        HeavyWeightConfigValueProxy* by_default(string val);
        HeavyWeightConfigValueProxy* by_default(int val);
        // END MODIF cache_value_proxy
        bool is_inverted();


        friend class ConfigCache;
        friend class Config;
        // BEGIN MODIF cache_value_proxy
        friend class HeavyWeightConfigValueProxy;
        // END MODIF cache_value_proxy
        friend class ConfigSource;
        friend class Configurator;
        friend class FileConfigSource;

    private:
        // BEGIN MODIF cache_value_proxy
//        bool has_characters( const char* mask );
        string value;
//        int default_int;
//        float default_double;
        uint16_t check_sums[3];
//        bool found;
//        bool default_set
        // END MODIF cache_value_proxy
};








#endif

#ifndef FUNCTION_INTERPOLATION_H
#define FUNCTION_INTERPOLATION_H

#include <set>
#include <utility>

class FunctionInterpolation {
public:
    FunctionInterpolation();
    void set_point(float x, float y);
    bool has_point(float x);
    void remove_point(float x);
    float get_value(float x);
    void clear();
    std::set<std::pair<float,float>, bool(*)(std::pair<float,float>,std::pair<float,float>)> points;
private:
};

#endif  // FUNCTION_INTERPOLATION_H

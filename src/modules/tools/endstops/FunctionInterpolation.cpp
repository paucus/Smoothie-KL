#include "FunctionInterpolation.h"


static bool fncomp(std::pair<float,float> v1,std::pair<float,float> v2) {
    return v1.first < v2.first;
}


FunctionInterpolation::FunctionInterpolation() : points(fncomp) {
}

void FunctionInterpolation::set_point(float x, float y){
    std::pair<float, float> search_val;
    search_val.first = x;
    std::set<std::pair<float,float>, bool(*)(std::pair<float,float>,std::pair<float,float>)>::iterator it = points.lower_bound(search_val);

    if (it != points.end() && (*it).first == x) {
        // delete and re-insert (the iterator pointer is read only)
        points.erase(it);
    }
    // entry doesn't exist. insert
    std::pair<float, float> v;
    v.first = x;
    v.second = y;
    points.insert(v);
}
void FunctionInterpolation::remove_point(float x){
    std::pair<float, float> search_val;
    search_val.first = x;
    std::set<std::pair<float,float>, bool(*)(std::pair<float,float>,std::pair<float,float>)>::iterator it = points.lower_bound(search_val);

    if (it != points.end() && (*it).first == x) {
        points.erase(it);
    }
}
bool FunctionInterpolation::has_point(float x){
    std::pair<float, float> search_val;
    search_val.first = x;
    std::set<std::pair<float,float>, bool(*)(std::pair<float,float>,std::pair<float,float>)>::iterator it = points.lower_bound(search_val);

    return (it != points.end() && (*it).first == x);
}
void FunctionInterpolation::clear(){
    points.clear();
}

float FunctionInterpolation::get_value(float x){
    if (points.size() == 0) {
        return 0;
    } else if (x <= (*(points.begin())).first) {
        // if lower than the first point, return the first point
        return (*(points.begin())).second;
    } else if (x >= (*(points.rbegin())).first) {
        // if higher than the last point, return the last point
        return (*(points.rbegin())).second;
    } else {
        std::pair<float, float> last_value;
        for (std::set<std::pair<float,float>, bool(*)(std::pair<float,float>,std::pair<float,float>)>::iterator it = points.begin(); it != points.end(); it++) {
            std::pair<float, float> point = *it;
            if (x == point.first) {
                return point.second;
            } else if (x < point.first) {
                // interpolate
                return (x - point.first) * (last_value.second - point.second) / (last_value.first - point.first) + point.second;
            }
            last_value = point;
        }
        return last_value.second;
    }
}

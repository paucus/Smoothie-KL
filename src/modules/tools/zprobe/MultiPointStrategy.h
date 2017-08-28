#ifndef _MULTIPOINTSTRATEGY
#define _MULTIPOINTSTRATEGY

#include "LevelingStrategy.h"

#include <string.h>
#include <tuple>
// BEGIN MODIF autolevel
#include "modules/tools/autolevel/vector_3.h"
// END MODIF autolevel

#define multi_point_leveling_strategy_checksum CHECKSUM("multi-point-leveling")

class StreamOutput;
class Plane3D;

class MultiPointStrategy : public LevelingStrategy
{
public:
    MultiPointStrategy(ZProbe *zprobe);
    ~MultiPointStrategy();
    bool handleGcode(Gcode* gcode);
    bool handleConfig();
    float getZOffset(float x, float y);

private:
    void homeXY();
    bool doProbing(StreamOutput *stream);
    std::tuple<float, float> parseXY(const char *str);
    std::tuple<float, float, float> parseXYZ(const char *str);
    void setAdjustFunction(bool);

    std::tuple<float, float, float> probe_offsets;
    std::tuple<float, float> boundary_points[2];
    Plane3D *plane;
    struct {
        bool home:1;
        bool save:1;
    };
    float tolerance;
    // BEGIN MODIF autolevel
    vector_3 rotation_vertex;
    vector_3 rotation_vertex_virtual;
    int num_x_points;
    int num_y_points;
    float lift_z_distance;
    float lift_to_improve_z_measure;
    // END MODIF autolevel
};

#endif

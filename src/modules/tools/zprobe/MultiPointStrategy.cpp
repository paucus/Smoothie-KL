/*
    Author: Jim Morris (wolfmanjm@gmail.com) of ThreePointStrategy, modified by Kikai Labs to adapt it to a
    multi point strategy.
    License: GPL3 or better see <http://www.gnu.org/licenses/>

    Based on Three point strategy

    Summary
    -------
    Probes an NxM user specified grid on the bed and determines the plane of the bed relative to the probe.
    As the head moves, it will adapt the x,y,z values to rotate the whole bed. The main difference with
    ThreePointStrategy, is that the other it only adapts Z, while MultiPointStrategy changes all the
    dimensions. For high slopes, this is quite important.

    Configuration
    -------------
    The strategy must be enabled in the config as well as zprobe.

    leveling-strategy.multi-point-leveling.enable         true

    Two probe points must be defined, which define a rectangle where the NxM grid lays. The points are taken
    at a uniform distance from each other.
    They can be defined in the config file as:-

    leveling-strategy.multi-point-leveling.point1         20.0,10.0   # the first probe point (x,y)
    leveling-strategy.multi-point-leveling.point2         280.0,190.0 # the second probe point (x,y)

    or they may be defined (and saved with M500) using M557 P0 X30 Y40.5  where P is 0,1

    probe offsets from the nozzle or tool head can be defined with

    leveling-strategy.multi-point-leveling.probe_offsets  0,0,0  # probe offsetrs x,y,z

    they may also be set with M565 X0 Y0 Z0
    Apart from this, the Z distance from bed correction is considered (calling delta_mm_from_bed).

    To force homing in X and Y before G32 does the probe the following can be set in config, this is the default

    leveling-strategy.multi-point-leveling.home_first    true   # disable by setting to false

    The probe tolerance can be set using the config line

    leveling-strategy.multi-point-leveling.tolerance   0.03    # the probe tolerance in mm, default is 0.03mm


    Usage
    -----
    G32 probes the NxM grid points and defines the bed plane, this will remain in effect until reset or M561
    G31 reports the status

    M557 defines the probe points
    M561 clears the plane and the bed leveling is disabled until G32 is run again
    M565 defines the probe offsets from the nozzle or tool head

    M500 saves the probe points and the probe offsets
    M503 displays the current settings
*/

#include "MultiPointStrategy.h"
#include "Kernel.h"
#include "Config.h"
#include "Robot.h"
#include "StreamOutputPool.h"
#include "Gcode.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "PublicDataRequest.h"
#include "PublicData.h"
#include "Conveyor.h"
#include "ZProbe.h"
#include "Plane3D.h"
#include "nuts_bolts.h"

#include <string>
#include <algorithm>
#include <cstdlib>
#include <cmath>

#define probe_point_1_checksum       CHECKSUM("point1")
#define probe_point_2_checksum       CHECKSUM("point2")
#define probe_point_3_checksum       CHECKSUM("point3")
#define probe_offsets_checksum       CHECKSUM("probe_offsets")
#define home_checksum                CHECKSUM("home_first")
#define tolerance_checksum           CHECKSUM("tolerance")
#define save_plane_checksum          CHECKSUM("save_plane")
// BEGIN MODIF autolevel
#define autolevel_rotation_vertex_x_checksumm                   CHECKSUM("autolevel_rotation_vertex_x")
#define autolevel_rotation_vertex_y_checksumm                   CHECKSUM("autolevel_rotation_vertex_y")
#define autolevel_rotation_vertex_z_checksumm                   CHECKSUM("autolevel_rotation_vertex_z")
#define autolevel_rotation_vertex_virtual_x_checksumm           CHECKSUM("autolevel_rotation_vertex_virtual_x")
#define autolevel_rotation_vertex_virtual_y_checksumm           CHECKSUM("autolevel_rotation_vertex_virtual_y")
#define autolevel_rotation_vertex_virtual_z_checksumm           CHECKSUM("autolevel_rotation_vertex_virtual_z")
#define num_x_points_checksum                                   CHECKSUM("num_x_points")
#define num_y_points_checksum                                   CHECKSUM("num_y_points")
#define z_safe_homing_position_checksum                         CHECKSUM("z_safe_homing_position")
#define lift_to_improve_z_measure_checksum                                     CHECKSUM("z_debounce")

#include "AutoLevelStatus.h"
#include "qr_solve.h"
#include "EndstopsPublicAccess.h"
#include "utils.h"
// END MODIF autolevel
// BEGIN MODIF correct_nozzle_distance_to_bed
#include "StepperMotor.h"
// END MODIF correct_nozzle_distance_to_bed
// BEGIN MODIF limit_endstops
#define STEPPER THEKERNEL->robot->actuators
// END MODIF limit_endstops

MultiPointStrategy::MultiPointStrategy(ZProbe *zprobe) : LevelingStrategy(zprobe)
{
    for (int i = 0; i < 2; ++i) {
        boundary_points[i] = std::make_tuple(NAN, NAN);
    }
    plane = nullptr;
    // BEGIN MODIF autolevel
    lift_z_distance = 0;
    tolerance = 0.0;
    num_x_points = 3;
    num_y_points = 4;
    // END MODIF autolevel
}

MultiPointStrategy::~MultiPointStrategy()
{
    delete plane;
}

bool MultiPointStrategy::handleConfig()
{
    // format is xxx,yyy for the probe points
    std::string p1 = THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, probe_point_1_checksum)->by_default("")->as_string();
    std::string p2 = THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, probe_point_2_checksum)->by_default("")->as_string();
    if(!p1.empty()) boundary_points[0] = parseXY(p1.c_str());
    if(!p2.empty()) boundary_points[1] = parseXY(p2.c_str());

    // Probe offsets xxx,yyy,zzz
    std::string po = THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, probe_offsets_checksum)->by_default("0,0,0")->as_string();
    this->probe_offsets= parseXYZ(po.c_str());

    this->home= THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, home_checksum)->by_default(true)->as_bool();
    this->tolerance= THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, tolerance_checksum)->by_default(0.03F)->as_number();
    this->save= THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, save_plane_checksum)->by_default(false)->as_bool();
    // BEGIN MODIF autolevel
    // Shift the central position according to the autolevel rotation vertex
    rotation_vertex.x = THEKERNEL->config->value(autolevel_rotation_vertex_x_checksumm)->by_default(178)->as_number();
    rotation_vertex.y = THEKERNEL->config->value(autolevel_rotation_vertex_y_checksumm)->by_default(100)->as_number();
    rotation_vertex.z = THEKERNEL->config->value(autolevel_rotation_vertex_z_checksumm)->by_default(0)->as_number();
    rotation_vertex_virtual.x = THEKERNEL->config->value(autolevel_rotation_vertex_virtual_x_checksumm)->by_default(150)->as_number();
    rotation_vertex_virtual.y = THEKERNEL->config->value(autolevel_rotation_vertex_virtual_y_checksumm)->by_default(100)->as_number();
    rotation_vertex_virtual.z = THEKERNEL->config->value(autolevel_rotation_vertex_virtual_z_checksumm)->by_default(0)->as_number();
    num_x_points = THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, num_x_points_checksum )->by_default( 3 )->as_int();
    num_y_points = THEKERNEL->config->value(leveling_strategy_checksum, multi_point_leveling_strategy_checksum, num_y_points_checksum )->by_default( 4 )->as_int();
    // For compatibility, let's keep the configuration property name.
    lift_z_distance = THEKERNEL->config->value( z_safe_homing_position_checksum )->by_default( 0 )->as_number();
    lift_to_improve_z_measure = THEKERNEL->config->value( leveling_strategy_checksum, multi_point_leveling_strategy_checksum, lift_to_improve_z_measure_checksum )->by_default( 2 )->as_number();
    // END MODIF autolevel
    return true;
}

bool MultiPointStrategy::handleGcode(Gcode *gcode)
{
    if(gcode->has_g) {
        // G code processing
        if( gcode->g == 31 ) { // report status
            if(this->plane == nullptr) {
                 gcode->stream->printf("Bed leveling plane is not set\n");
            }else{
                 gcode->stream->printf("Bed leveling plane normal= %f, %f, %f\n", plane->getNormal()[0], plane->getNormal()[1], plane->getNormal()[2]);
            }
            gcode->stream->printf("Probe is %s\n", zprobe->getProbeStatus() ? "Triggered" : "Not triggered");
            return true;

        // BEGIN MODIF autolevel
        } else if( gcode->g == 32 || gcode->g == 28 ) { // multi point probe
            // END MODIF autolevel
            // BEGIN MODIF limit_endstops
            // reset limits (so that the printer can move freely before measuring the probe height)
            STEPPER[Z_AXIS]->set_max_position_steps(MAX_POSITION_STEPS);
            // END MODIF limit_endstops
            // first wait for an empty queue i.e. no moves left
            THEKERNEL->conveyor->wait_for_empty_queue();
            // BEGIN MODIF autolevel
            /*
            if(!gcode->has_letter('K')) { // K will keep current compensation to test plane
                // clear any existing plane and compensation
                delete this->plane;
                this->plane= nullptr;
                setAdjustFunction(false);
            }
            */
            if(this->plane) delete this->plane;
            this->plane= nullptr;
            setAdjustFunction(false);
            // END MODIF autolevel

            if(!doProbing(gcode->stream)) {
                gcode->stream->printf("Probe failed to complete, probe not triggered or other error\n");
            } else {
                gcode->stream->printf("Probe completed, bed plane defined\n");
            }
            // BEGIN MODIF limit_endstops
            // reset limits (so that the printer can move freely after the whole leveling process)
            STEPPER[Z_AXIS]->set_max_position_steps(MAX_POSITION_STEPS);
            // END MODIF limit_endstops
            return true;
        }

    } else if(gcode->has_m) {
        if(gcode->m == 557) { // M557 - set probe points eg M557 P0 X30 Y40.5  where P is 0,1,2
            int idx = 0;
            float x = NAN, y = NAN;
            if(gcode->has_letter('P')) idx = gcode->get_value('P');
            if(gcode->has_letter('X')) x = gcode->get_value('X');
            if(gcode->has_letter('Y')) y = gcode->get_value('Y');
            if(idx >= 0 && idx <= 1) {
                boundary_points[idx] = std::make_tuple(x, y);
            }else{
                gcode->stream->printf("only 2 boundary points allowed P0-P1\n");
            }
            return true;

        } else if(gcode->m == 561) { // M561: Set Identity Transform with no parameters, set the saved plane if A B C D are given
            delete this->plane;
            if(gcode->get_num_args() == 0) {
                this->plane= nullptr;
                // delete the compensationTransform in robot
                setAdjustFunction(false);
            }else{
                // smoothie specific way to restire a saved plane
                // BEGIN MODIF autolevel
//                uint32_t a,b,c,d;
                float a,b,c,d;
                a=b=c=d= 0;
                if(gcode->has_letter('A')) a = gcode->get_value('A');
                if(gcode->has_letter('B')) b = gcode->get_value('B');
                if(gcode->has_letter('C')) c = gcode->get_value('C');
                if(gcode->has_letter('D')) d = gcode->get_value('D');
                this->plane= new Plane3D(a, b, c, d);
                // END MODIF autolevel
                setAdjustFunction(true);
            }
            return true;

        } else if(gcode->m == 565) { // M565: Set Z probe offsets
            float x= 0, y= 0, z= 0;
            if(gcode->has_letter('X')) x = gcode->get_value('X');
            if(gcode->has_letter('Y')) y = gcode->get_value('Y');
            if(gcode->has_letter('Z')) z = gcode->get_value('Z');
            probe_offsets = std::make_tuple(x, y, z);
            return true;

        } else if(gcode->m == 500 || gcode->m == 503) { // M500 save, M503 display
            float x, y, z;
            gcode->stream->printf(";Probe points:\n");
            for (int i = 0; i < 2; ++i) {
                std::tie(x, y) = boundary_points[i];
                gcode->stream->printf("M557 P%d X%1.5f Y%1.5f\n", i, x, y);
            }
            gcode->stream->printf(";Probe offsets:\n");
            std::tie(x, y, z) = probe_offsets;
            gcode->stream->printf("M565 X%1.5f Y%1.5f Z%1.5f\n", x, y, z);

            // encode plane and save if set and M500 and enabled
            if(this->save && this->plane != nullptr) {
                if(gcode->m == 500) {
                    // BEGIN MODIF autolevel
//                    uint32_t a, b, c, d;
                    float a, b, c, d;
                    this->plane->encode(a, b, c, d);
                    gcode->stream->printf(";Saved bed plane:\nM561 A%.8f B%.8f C%.8f D%.8f \n", a, b, c, d);
//                    gcode->stream->printf(";Saved bed plane:\nM561 A%lu B%lu C%lu D%lu \n", a, b, c, d);
                    // END MODIF autolevel
                }else{
                    gcode->stream->printf(";The bed plane will be saved on M500\n");
                }
            }
            return true;

        // BEGIN MODIF autolevel
        } else if (gcode->m == 734) {    // M734 get or set transformation matrix
            if(this->plane != nullptr) {
                THEKERNEL->conveyor->wait_for_empty_queue();
                Vector3 v = plane->getNormal();
                gcode->stream->printf("Normal plane: %1.4f, %1.4f, %1.4f\n", v[X_AXIS], v[Y_AXIS], v[Z_AXIS]);
                gcode->stream->printf("Slope: %1.8f, %1.8f\n", v[X_AXIS] / v[Z_AXIS], v[Y_AXIS] / v[Z_AXIS]);
                matrix_3x3 transformation = matrix_3x3::create_look_at(vector_3(v[X_AXIS], v[Y_AXIS], v[Z_AXIS]));
                transformation.debug("Transformation matrix");
            } else {
                gcode->stream->printf("No autolevel set\n");
            }
        // END MODIF autolevel
        }
    }

    return false;
}

void MultiPointStrategy::homeXY()
{
    // BEGIN MODIF autolevel
    // There's no risk of entering a recursive loop because G28 X0/Y0 is not handled by this class, but by Endstop class instead.
    // END MODIF autolevel
    Gcode gc("G28 X0 Y0", &(StreamOutput::NullStream));
    THEKERNEL->call_event(ON_GCODE_RECEIVED, &gc);
}

// BEGIN MODIF autolevel
bool MultiPointStrategy::doProbing(StreamOutput *stream)
{
    float x, y;
    // check the probe points have been defined
    for (int i = 0; i < 2; ++i) {
        std::tie(x, y) = boundary_points[i];
        if(isnan(x) || isnan(y)) {
            stream->printf("Probe point P%d has not been defined, use M557 P%d Xnnn Ynnn to define it\n", i, i);
            return false;
        }
    }

    // BEGIN MODIF autolevel
    // notify beginning of autolevel
    autolevel_status_change_t status;
    status.event = al_begin;
    THEKERNEL->call_event(ON_AUTOLEVEL_STATUS_CHANGE, &status);
    // END MODIF autolevel

    // BEGIN MODIF autolevel
    // lift Z before starting
    zprobe->coordinated_move(NAN, NAN, THEKERNEL->robot->last_milestone[Z_AXIS] + lift_z_distance, zprobe->getFastFeedrate());
    // END MODIF autolevel

    // optionally home XY axis first, but allow for manual homing
    if(this->home)
        homeXY();

    // move to the first probe point
    std::tie(x, y) = boundary_points[0];
    // offset by the probe XY offset
    x -= std::get<X_AXIS>(this->probe_offsets);
    y -= std::get<Y_AXIS>(this->probe_offsets);
    zprobe->coordinated_move(this->rotation_vertex[X_AXIS], this->rotation_vertex[Y_AXIS], NAN, zprobe->getFastFeedrate());

    // for now we use probe to find bed and not the Z min endstop
    // the first probe point becomes Z == 0 effectively so if we home Z or manually set z after this, it needs to be at the first probe point

    // TODO this needs to be configurable to use min z or probe

    // find bed via probe
    int s;
    // BEGIN MODIF autolevel
    float* fast_rates = get_public_data_ptr<float>(endstops_checksum, fast_rates_checksum, 0, nullptr);
    // Go down very fast to speed the homing up. This doesn't need to be a very exact measure
    if(!zprobe->run_probe_feed(s, fast_rates[Z_AXIS])) return false;
    // Go up a bit and go down slower to take a more exact measure
    zprobe->coordinated_move(NAN, NAN, THEKERNEL->robot->last_milestone[Z_AXIS] + lift_to_improve_z_measure, zprobe->getFastFeedrate());
    float* slow_rates = get_public_data_ptr<float>(endstops_checksum, slow_rates_checksum, 0, nullptr);
    if(!zprobe->run_probe_feed(s, slow_rates[Z_AXIS])) return false;
    // END MODIF autolevel

    // TODO if using probe then we probably need to set Z to 0 at first probe point, but take into account probe offset from head
    // BEGIN MODIF correct_nozzle_distance_to_bed
    THEKERNEL->robot->reset_axis_position(std::get<Z_AXIS>(this->probe_offsets) + zprobe->delta_mm_from_bed(), Z_AXIS);

    // move up to specified probe start position
    zprobe->coordinated_move(NAN, NAN, zprobe->getProbeHeight() + zprobe->delta_mm_from_bed(), zprobe->getSlowFeedrate()); // move to probe start position
    // END MODIF correct_nozzle_distance_to_bed

    // probe the N points
    int no_of_measures = num_x_points * num_y_points;

    vector_3* v = new vector_3[no_of_measures];
    for (int j = 0; j < num_y_points; j++) {
        int from = j % 2;
        int to = 1 - from;
        for (int i = 0; i < num_x_points; i++) {

            x = (std::get<X_AXIS>(boundary_points[to]) * i + std::get<X_AXIS>(boundary_points[from]) * (num_x_points - i - 1)) / (num_x_points - 1);
            y = (std::get<Y_AXIS>(boundary_points[1]) * j + std::get<Y_AXIS>(boundary_points[0]) * (num_y_points - j - 1)) / (num_y_points - 1);

            // offset moves by the probe XY offset
            float z = zprobe->probeDistance(x-std::get<X_AXIS>(this->probe_offsets), y-std::get<Y_AXIS>(this->probe_offsets));
            if(isnan(z)) return false; // probe failed
            // BEGIN MODIF correct_nozzle_distance_to_bed
            z = zprobe->getProbeHeight() + zprobe->delta_mm_from_bed() - z; // relative distance between the probe points, lower is negative z
            // END MODIF correct_nozzle_distance_to_bed
            stream->printf("DEBUG: P%d:%1.4f\n", i + j * num_y_points, z);
            v[i + j * num_x_points] = vector_3(x, y, z);
        }
    }

    // BEGIN MODIF correct_nozzle_distance_to_bed
    // REMOVED: This doesn't apply to us.
    // if first point is not within tolerance report it, it should ideally be 0
    /*if(abs(v[0][2]) > this->tolerance) {
        stream->printf("WARNING: probe is not within tolerance: %f > %f\n", abs(v[0][2]), this->tolerance);
    }*/
    // END MODIF correct_nozzle_distance_to_bed

    // estimate the plate normal with least squares
    // a X + b Y + c = Z
    // calculate a, b and c
    double * matrix = new double[no_of_measures * 3];
    double * result = new double[no_of_measures];
    for (int i = 0; i < no_of_measures; i++) {
        matrix[i + 0 * no_of_measures] = v[i].x;
        matrix[i + 1 * no_of_measures] = v[i].y;
        matrix[i + 2 * no_of_measures] = 1;

        result[i] = v[i].z;
    }
    double * solution = qr_solve(no_of_measures, 3, matrix, result );

    delete[] matrix;
    delete[] result;

    // define the plane
    delete this->plane;


    // check tolerance level here default 0.03mm
//    auto mm = std::minmax({v[0][2], v[1][2], v[2][2]}); ESTO NO LO DEBERIA HACER CON ESTOS TRES PUNTOS SOLOS, SINO CON TODOS - CAMBIA TABS POR ESPACIOS
//    if((mm.second - mm.first) <= this->tolerance) {
//        this->plane= nullptr; // plane is flat no need to do anything
//        stream->printf("DEBUG: flat plane\n");
//        // clear the compensationTransform in robot
//        setAdjustFunction(false);
//
//    }else{
    // The plane can be described this way:
    // Z = a X + b Y + c
    // Displacing it to the 0,0,0
    // Z = a X + b Y -> a X + b Y - Z = 0 -> [a b -1]*transpose([X Y Z]) = 0
    // So, [a b -1] is the plane normal vector
    float a = -solution[0];
    float b = -solution[1];
    float c = 1.0;
    float d = this->rotation_vertex[X_AXIS] * a + this->rotation_vertex[Y_AXIS] * b;
    this->plane = new Plane3D(a, b, c, d);
    stream->printf("DEBUG: plane normal= %f, %f, %f\n", plane->getNormal()[0], plane->getNormal()[1], plane->getNormal()[2]);
    setAdjustFunction(true);
//    }

    // free memory from equation resolution
    delete[] v;
    free(solution);

    // BEGIN MODIF autolevel
    // Go home
    bool end_safe_homing_at_center_of_heatbed = get_public_data_val<bool>(end_safe_homing_at_center_of_heatbed_checksum, 0, 0, false);
    if (end_safe_homing_at_center_of_heatbed) {
        zprobe->coordinated_move(this->rotation_vertex[X_AXIS], this->rotation_vertex[Y_AXIS], 0, zprobe->getFastFeedrate());
    } else {
        zprobe->coordinated_move(0, 0, 0, zprobe->getFastFeedrate());
    }

    // update known positions
    bool* axis_known_positions = get_public_data_ptr<bool>(endstops_checksum, axis_position_known_checksum, 0, nullptr);
    if (axis_known_positions) {
        for (int i = X_AXIS; i <= Z_AXIS; i++) axis_known_positions[i] = true;
    }

    // notify end of autolevel
    status.event = al_end;
    THEKERNEL->call_event(ON_AUTOLEVEL_STATUS_CHANGE, &status);

    // Print final position, so that repetier or other clients that depend on this get to know it.
    float robot_position[3];
    THEKERNEL->robot->get_axis_position(robot_position);
    stream->printf("ok");
    for (char c = 'X'; c <= 'Z'; c++)
        stream->printf(" %c:%f", c, robot_position[c-'X']);
    stream->printf("\r\n");
    // END MODIF autolevel

    return true;
}
// END MODIF autolevel

void MultiPointStrategy::setAdjustFunction(bool on)
{
    if(on) {
        // set the compensationTransform in robot
        // BEGIN MODIF autolevel
        THEKERNEL->robot->compensationTransform= [this](float target[3]) {
//            target[2] += this->plane->getz(target[0], target[1]);
            vector_3 t(target);
            float* nd = this->plane->getNormal().data();
            vector_3 n(nd);
            if (n.get_length() < 1E-7) {
                // We can assume it is zero. Use default plane.
                n = vector_3(0,0,1);
            } else {
                n.normalize();
                if (n.z < 0) n *= -1;
            }
            // The virtual axis coordinates can be obtained from the plane.
            // If the plane normal is N=(Nx, Ny, Nz), then
            // Vx = (Nz, 0, -Nx)
            // Vy = (0, Nz, -Nx)
            // Vz = (Nx, Ny, Nz)
            // Vvirt = vr + [ Vx Vy Vz ] * (target - r)
            vector_3 dt = t - rotation_vertex_virtual;
            vector_3 ax(n[Z_AXIS], 0, -n[X_AXIS]);
            ax.normalize();
            vector_3 ay(0, n[Z_AXIS], -n[Y_AXIS]);
            ay.normalize();

            vector_3 p = rotation_vertex + ax * dt.x + ay * dt.y + n * dt.z;
            target[X_AXIS] = p.x;
            target[Y_AXIS] = p.y;
            target[Z_AXIS] = p.z;
        };
        THEKERNEL->robot->slopeTransform = [this](struct Robot::slope_pt p, float* slope) {
        	*slope = this->plane->getz(p.x, p.y) - this->plane->getz(0, 0);
        };
        // END MODIF autolevel
    }else{
        // clear it
        // BEGIN MODIF safe_homing
        // change the zero to make it match the same zero
//        THEKERNEL->robot->compensationTransform= nullptr;
        THEKERNEL->robot->compensationTransform = [this] (float target[3]) {
            vector_3 d = rotation_vertex - rotation_vertex_virtual;
            for (int i = 0; i < 3; ++i) {
                target[i] += d[i];
            }
        };
        // END MODIF safe_homing
        // BEGIN MODIF autolevel
        THEKERNEL->robot->slopeTransform = nullptr;
        // END MODIF autolevel
    }

    // BEGIN MODIF autolevel
    memcpy(THEKERNEL->robot->transformed_last_milestone, THEKERNEL->robot->last_milestone, sizeof(THEKERNEL->robot->last_milestone));
    THEKERNEL->robot->compensationTransform(THEKERNEL->robot->transformed_last_milestone);
    // END MODIF autolevel
}

// find the Z offset for the point on the plane at x, y
float MultiPointStrategy::getZOffset(float x, float y)
{
    if(this->plane == nullptr) return NAN;
    return this->plane->getz(x, y);
}

// parse a "X,Y" string return x,y
std::tuple<float, float> MultiPointStrategy::parseXY(const char *str)
{
    float x = NAN, y = NAN;
    char *p;
    x = strtof(str, &p);
    if(p + 1 < str + strlen(str)) {
        y = strtof(p + 1, nullptr);
    }
    return std::make_tuple(x, y);
}

// parse a "X,Y,Z" string return x,y,z tuple
std::tuple<float, float, float> MultiPointStrategy::parseXYZ(const char *str)
{
    float x = 0, y = 0, z= 0;
    char *p;
    x = strtof(str, &p);
    if(p + 1 < str + strlen(str)) {
        y = strtof(p + 1, &p);
        if(p + 1 < str + strlen(str)) {
            z = strtof(p + 1, nullptr);
        }
    }
    return std::make_tuple(x, y, z);
}

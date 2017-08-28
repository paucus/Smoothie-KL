/*
 * KisslicerDialect.h
 *
 *  Created on: Feb 1, 2016
 *      Author: eai
 */

#ifndef KISSLICERDIALECT_H_
#define KISSLICERDIALECT_H_

#include "SlicerDialect.h"

/*
; KISSlicer - FREE
; Linux
; version 1.1.0.14
; Built: May  8 2013, 11:56:28
; Running on 4 cores
;
; Saved: Fri Jan  9 17:24:58 2015
; 'bishopfinal.gcode'
;
; *** Printer Settings ***
;
; printer_name = sample printer
; bed_STL_filename =
; extension = gcode
; cost_per_hour = 0
; g_code_prefix = 3B205B6D6D5D206D6F64650A4732310A3B206162736F
;     6C757465206D6F64650A4739300A
; g_code_warm = 3B2053656C6563742065787472756465722C207761726D
;     2C2070757267650A0A3B204266422D7374796C650A4D3C4558542B31
;     3E303420533C54454D503E0A4D3534320A4D35353C4558542B313E20
;     50333230303020533930300A4D3534330A0A3B2035442D7374796C65
;     0A543C4558542B303E0A4D31303920533C54454D503E0A
; g_code_cool = 3B2047756172616E746565642073616D65206578747275
;     6465722C20636F6F6C696E6720646F776E0A0A3B204266422D737479
;     6C650A4D3C4558542B313E303420533C54454D503E0A0A3B2035442D
;     7374796C650A4D31303420533C54454D503E0A
; g_code_N_layers = 3B204D617962652072652D686F6D65205820262059
;     3F
; g_code_postfix = 3B20416C6C207573656420657874727564657273206
;     1726520616C72656164792027436F6F6C65642720746F20300A
; post_process = NULL
; every_N_layers = 0
; num_extruders = 1
; firmware_type = 1
; add_comments = 1
; fan_on = M106
; fan_off = M107
; fan_pwm = 1
; add_m101_g10 = 0
; z_speed_mm_per_s = 3.5
; z_settle_mm = 0.25
; bed_size_x_mm = 200
; bed_size_y_mm = 200
; bed_size_z_mm = 200
; bed_offset_x_mm = 10
; bed_offset_y_mm = 10
; bed_offset_z_mm = 0
; bed_roughness_mm = 0.25
; travel_speed_mm_per_s = 500
; first_layer_speed_mm_per_s = 10
; dmax_per_layer_mm_per_s = 50
; xy_accel_mm_per_s_per_s = 1500
; lo_speed_perim_mm_per_s = 5
; lo_speed_solid_mm_per_s = 15
; lo_speed_sparse_mm_per_s = 30
; hi_speed_perim_mm_per_s = 15
; hi_speed_solid_mm_per_s = 60
; hi_speed_sparse_mm_per_s = 75
; ext_gain_1 = 1
; ext_material_1 = 0
; ext_axis_1 = 0
; ext_gain_2 = 1
; ext_material_2 = 0
; ext_axis_2 = 0
; ext_gain_3 = 1
; ext_material_3 = 0
; ext_axis_3 = 0
; model_ext = 0
; support_ext = 0
; support_body_ext = 0
; raft_ext = 0
; solid_loop_overlap_fraction = 0.5
;
; *** Material Settings for Extruder 1 ***
;
; material_name = sample material
; g_code_matl = 3B204D617962652073657420736F6D65206D6174657269
;     616C2D737065636966696320472D636F64653F
; fan_Z_mm = 0
; fan_loops_percent = 100
; fan_inside_percent = 0
; fan_cool_percent = 100
; temperature_C = 250
; keep_warm_C = 180
; first_layer_C = 255
; bed_C = 80
; sec_per_C_per_C = 0
; flow_min_mm3_per_s = 0.01
; flow_max_mm3_per_s = 10
; destring_suck = 1.25
; destring_prime = 1.25
; destring_min_mm = 1
; destring_trigger_mm = 100
; destring_speed_mm_per_s = 15
; Z_lift_mm = 0
; min_layer_time_s = 10
; wipe_mm = 10
; cost_per_cm3 = 0
; flowrate_tweak = 1
; fiber_dia_mm = 3
; color = 0
;
; *** Style Settings ***
;
; style_name = sample style
; layer_thickness_mm = 0.1
; extrusion_width_mm = 0.5
; num_loops = 3
; skin_thickness_mm = 1.2
; infill_extrusion_width = 0.5
; infill_density_denominator = 4
; stacked_layers = 1
; use_destring = 1
; use_wipe = 1
; loops_insideout = 0
; infill_st_oct_rnd = 1
; inset_surface_xy_mm = 0
; seam_jitter_degrees = 0
; seam_depth_scaler = 1
;
; *** Support Settings ***
;
; support_name = sample support
; support_sheathe = 0
; support_density = 3
; support_inflate_mm = 0
; support_gap_mm = 0.5
; support_angle_deg = 45
; support_z_max_mm = -1
; sheathe_z_max_mm = -1
; raft_mode = 0
; prime_pillar_mode = 0
; raft_inflate_mm = 2
;
; *** Actual Slicing Settings As Used ***
;
; layer_thickness_mm = 0.1
; extrusion_width = 0.5
; num_ISOs = 3
; wall_thickness = 1.2
; infill_style = 5
; support_style = 3
; support_angle = 44.9
; destring_min_mm = 1
; stacked_infill_layers = 1
; raft_style = 0
; extra_raft_depth = 0.25
; oversample_res_mm = 0.125
; crowning_threshold_mm = 1
; loops_insideout = 0
; solid_loop_overlap_fraction = 0.5
; inflate_raft_mm = 0
; inflate_support_mm = 0
; model_support_gap_mm = 0.5
; infill_st_oct_rnd = 1
; support_Z_max_mm = 1e+20
; sheathe_Z_max_mm = 0
; inset_surface_xy_mm = 0
; seam_jitter_degrees = 0
; seam_depth_scaler = 1
; Speed vs Quality = 0.00
; Perimeter Speed = 14.95
; Solid Speed = 59.78
; Sparse Speed = 74.78
;
; *** G-code Prefix ***
;
; [mm] mode


Footer:

; *** G-code Postfix ***
;
; All used extruders are already 'Cooled' to 0

;
;
;
; Estimated Build Time:   72.45 minutes
; Estimated Build Volume: 2.982 cm^3
; Estimated Build Cost:   $0.00
;
; *** Extrusion Time Breakdown ***
; * estimated time in [s]
; * before possibly slowing down for 'cool'
; * not including Z-travel
;	+-------------+-------------+-------------+-----------------------+
;	| Extruder #1 | Extruder #2 | Extruder #3 | Path Type             |
;	+-------------+-------------+-------------+-----------------------+
;	| 167.046     | 0           | 0           | Move                  |
;	| 0           | 0           | 0           | Pillar                |
;	| 0           | 0           | 0           | Raft                  |
;	| 134.203     > 0           > 0           > Support Interface     |
;	| 90.2172     | 0           | 0           | Support (may Stack)   |
;	| 902.807     | 0           | 0           | Perimeter             |
;	| 426.279     | 0           | 0           | Loop                  |
;	| 357.126     > 0           > 0           > Solid                 |
;	| 29.6994     | 0           | 0           | Sparse Infill         |
;	| 239.806     | 0           | 0           | Stacked Sparse Infill |
;	| 288.662     | 0           | 0           | Wipe (and De-string)  |
;	| 3.38181     > 0           > 0           > Crown                 |
;	| 0           | 0           | 0           | Prime Pillar          |
;	| 0           | 0           | 0           | Skirt                 |
;	| 0           | 0           | 0           | Pause                 |
;	| 0           > 0           > 0           > Extruder Warm-Up      |
;	+-------------+-------------+-------------+-----------------------+
; Total estimated (pre-cool) minutes: 43.99

*/

class KisslicerDialect : public SlicerDialect {
public:
	KisslicerDialect();
	virtual ~KisslicerDialect();

	void parse_line(SlicingInformation* info, const char* line);

	// Kisslicer has long headers. 7kbs should be enough.
	unsigned int get_header_len() {return 7*1024;};
	// It also has long footers, so, read 3kbs.
	unsigned int get_footer_len() {return 3*1024;};
	enum slicer_name get_slicer_name() {return KISSLICER;};

	static bool is_this_slicer_gcode(const char* line);
	static const unsigned int BYTES_TO_IDENTIFY_SLICER = 30; // Incomplete lines are discarded, so we must ensure the first line will be read
};

#endif /* KISSLICERDIALECT_H_ */

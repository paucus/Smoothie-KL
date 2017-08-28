#ifndef __ENDSTOPSPUBLICACCESS_H_
#define __ENDSTOPSPUBLICACCESS_H_

// addresses used for public data access
#define endstops_checksum    CHECKSUM("endstop")
#define trim_checksum        CHECKSUM("trim")
#define home_offset_checksum CHECKSUM("home_offset")
// BEGIN MODIF show_print
#define axis_position_known_checksum CHECKSUM("axis_position_known")
// END MODIF show_print
// BEGIN MODIF time_remaining_calculation
#define homing_position_checksum CHECKSUM("homing_position")
// END MODIF time_remaining_calculation
// BEGIN MODIF autolevel
#define fast_rates_checksum  CHECKSUM("fast_rates")
#define slow_rates_checksum  CHECKSUM("slow_rates")
// END MODIF autolevel
// BEGIN MODIF safe_homing
#define end_safe_homing_at_center_of_heatbed_checksum           CHECKSUM("end_safe_homing_at_center_of_heatbed")
// END MODIF safe_homing
#endif

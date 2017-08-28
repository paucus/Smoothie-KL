#pragma once

#define extruder_checksum                    CHECKSUM("extruder")
#define save_state_checksum                  CHECKSUM("save_state")
#define restore_state_checksum               CHECKSUM("restore_state")
#define target_checksum                      CHECKSUM("target")
#define target_position_checksum             CHECKSUM("target_position")
#define absolute_mode_checksum               CHECKSUM("absolute_mode")
// BEGIN MODIF extruder_multiplier
#define extruder_multiplier_checksum         CHECKSUM("extruder_multiplier")
// END MODIF extruder_multiplier

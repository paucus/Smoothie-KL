#ifndef LCD4DMODULECONSTANTS_H_
#define LCD4DMODULECONSTANTS_H_

typedef enum {
	TEXT_RIGHT, TEXT_CENTERED, TEXT_LEFT
} textalign_t;

typedef enum {
	SMALL, MEDIUM, BIG
} fontsize_t;

typedef enum {
	TEXT_CENTER_AROUND_X, TEXT_CENTER_FROM_X, TEXT_CENTER_NONE
} text_center_strategy_t;


#define PLA_BED_TEMPERATURE			filament_temperatures[0][1]
#define PLA_HOTEND_TEMPERATURE		filament_temperatures[0][0]
#define ABS_BED_TEMPERATURE			filament_temperatures[1][1]
#define ABS_HOTEND_TEMPERATURE		filament_temperatures[1][0]
#define NYLON_BED_TEMPERATURE		filament_temperatures[2][1]
#define NYLON_HOTEND_TEMPERATURE	filament_temperatures[2][0]
#define FLEX_BED_TEMPERATURE		filament_temperatures[3][1]
#define FLEX_HOTEND_TEMPERATURE		filament_temperatures[3][0]

#endif

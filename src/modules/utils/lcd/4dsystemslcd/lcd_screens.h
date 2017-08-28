/*
 * lcd_screens.h
 *
 *  Created on: Sep 3, 2015
 *      Author: eai
 */

#ifndef LCD_SCREENS_H_
#define LCD_SCREENS_H_


#include "DltPtr.h"
#include "LCD4DScreen.h"
#include "LCD4DAdjustmentsScreen.h"
#include "LCD4DAutolevelingScreen.h"
#include "LCD4DCalibrationScreen.h"
#include "LCD4DCancelConfirmationScreen.h"
#include "LCD4DChangeFilamentScreen.h"
#include "LCD4DInformationScreen.h"
#include "LCD4DLanguageScreen.h"
#include "LCD4DMainScreen.h"
#include "LCD4DManualControlScreen.h"
#include "LCD4DMessageScreen.h"
#include "LCD4DPauseScreen.h"
#include "LCD4DPrepareScreen.h"
#include "LCD4DPrintDoneScreen.h"
#include "LCD4DPrintingScreen.h"
#include "LCD4DScreenSaverScreen.h"
#include "LCD4DSelectModelScreen.h"
#include "LCD4DPrepareCustomTempScreen.h"
#include "LCD4DCriticalErrorScreen.h"
#include "LCD4DHeatingScreen.h"
#include "LCD4DPIDAutotuneScreen.h"
#include "LCD4DSettingsScreen.h"
#include "LCD4DNetworkSettingsScreen.h"
#include "LCD4DDisplaySettingsScreen.h"
#include "LCD4DCalibrationSettingsScreen.h"
#include "LCD4DAnalyzingFileScreen.h"
#include "LCD4DModelInfoScreen.h"
#include "LCD4DModelPreview.h"
#include "LCD4DPasswordScreen.h"
#include "LCD4DGenericConfirmationScreen.h"

// WARN: screens_available_s MUST only have DltPtr attributes (see lcd_clear_all_screens_except).
// If you need to make any of these windows persistent, add a line like
// make_persistent(lcd_screens.change_filament_screen);
// to lcd_initialize_counters.
typedef struct screens_available_s {
		DltPtr<LCD4DAdjustmentsScreen> adjustments_screen;
		DltPtr<LCD4DAutolevelingScreen> autoleveling_screen;
		DltPtr<LCD4DCalibrationScreen> calibration_screen;
		DltPtr<LCD4DCancelConfirmationScreen> cancel_confirmation_screen;
		DltPtr<LCD4DChangeFilamentScreen> change_filament_screen;
		DltPtr<LCD4DInformationScreen> config_screen;
		DltPtr<LCD4DLanguageScreen> language_screen;
		DltPtr<LCD4DMainScreen> main_screen;
		DltPtr<LCD4DManualControlScreen> manual_control_screen;
		DltPtr<LCD4DMessageScreen> message_screen;
		DltPtr<LCD4DPauseScreen> pause_screen;
		DltPtr<LCD4DPrepareScreen> prepare_screen;
		DltPtr<LCD4DPrintDoneScreen> printing_done_screen;
		DltPtr<LCD4DPrintingScreen> printing_screen;
		DltPtr<LCD4DScreenSaverScreen> screen_saver_screen;
		DltPtr<LCD4DSelectModelScreen> select_model_screen;
		DltPtr<LCD4DPrepareCustomTempScreen> custom_temperature_screen;
		DltPtr<LCD4DCriticalErrorScreen> critical_error_screen;
		DltPtr<LCD4DHeatingScreen> heating_screen;
		DltPtr<LCD4DPIDAutotuneScreen> pid_autotune_screen;
		DltPtr<LCD4DSettingsScreen> settings_screen;
		DltPtr<LCD4DNetworkSettingsScreen> network_settings_screen;
		DltPtr<LCD4DAnalyzingFileScreen> analyzing_file_screen;
		DltPtr<LCD4DDisplaySettingsScreen> display_settings_screen;
		DltPtr<LCD4DCalibrationSettingsScreen> calibration_settings_screen;
		DltPtr<LCD4DModelInfoScreen> model_info_screen;
		DltPtr<LCD4DModelPreview> model_preview_screen;
		DltPtr<LCD4DPasswordScreen> password_screen;
		DltPtr<LCD4DGenericConfirmationScreen> generic_confirmation_screen;
} screens_available_s;

extern screens_available_s lcd_screens;

void lcd_initialize_counters();
void lcd_clear_all_screens_except(LCD4DScreen* s);
int* lcd_get_counter(LCD4DScreen* s);

#endif /* LCD_SCREENS_H_ */

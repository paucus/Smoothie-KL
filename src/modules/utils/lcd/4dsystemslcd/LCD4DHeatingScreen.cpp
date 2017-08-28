/*
 * LCD4DHeatingScreen.cpp
 *
 *  Created on: Jun 25, 2015
 *      Author: idlt
 */

#include "LCD4DHeatingScreen.h"
#include "LCD4DModule.h"
#include "checksumm.h"
#include "PublicData.h"
#include "Pauser.h"
#include "modules/utils/player/PlayerPublicAccess.h"
#include "modules/tools/temperaturecontrol/TemperatureControlPublicAccess.h"

#include "LCD4DSelectModelScreen.h"

void LCD4DHeatingScreen::on_refresh() {
	LCD4DScreen::on_refresh();
}

int LCD4DHeatingScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	// Mark that we are not heating_cancelled
	heating_cancelled = false;

	lcd->img_Disable(handle, ALL);

	screen_print(0, 80, BIG, TEXT_CENTERED, KIKAIBLUE, translate(HEATING_BEFORE_PRINT));

	draw_lcd_images(ibPrepareCustomAccept, ibPrepareCustomAccept, handle);

	return 0;
}

static bool is_waiting_temperature_control(int temp_control_checksum){
	bool response;
	if (!PublicData::get_value(temperature_control_checksum, waiting_checksum, temp_control_checksum, &response)){
		// something failed
		return false;
	} else {
		return response;
	}
}
static void stop_waiting_temperature_control(int temp_control_checksum){
	bool false_var = false;
	if (!PublicData::set_value(temperature_control_checksum, waiting_checksum, temp_control_checksum, &false_var)){
		// something failed
	}
}
static void cancel_any_pending_heating(){
	uint16_t temp_controls[] = {hotend_checksum, bed_checksum};
	for (unsigned int i = 0; i < sizeof(temp_controls)/sizeof(uint16_t); i++) {
		if (is_waiting_temperature_control(temp_controls[i]))
			stop_waiting_temperature_control(temp_controls[i]);
	}
}
void LCD4DHeatingScreen::on_periodic_tick() {
	if (heating_cancelled) {
		// Periodically try to cancel. Heating has three steps:
		// * asynchronously heat the bed
		// * synchronously heat the hotend
		// * synchronously heat the bed
		// This ensures that once the process has finished, both the bed and hotend are at the
		// right temperature. We don't know in which of these three steps the cancel order is being
		// sent, so, we have to cancel "any" of these three. But heating_cancelled once is not enough,
		// because after heating_cancelled the synchronous hotend heat, the one of the bed takes place.
		// In order to fix this, we periodically cancel any heating process. Once heating is
		// finished or cancelled, the heating screen will go to the background, and it won't cancel
		// any more.
		cancel_any_pending_heating();
	}
}
int LCD4DHeatingScreen::process_click(int action, int button) {
	if (action == TOUCH_PRESSED) {
		for (int i = ibPrepareCustomAccept; i <= ibPrepareCustomAccept; i++) {
			lcd->img_SetWord(handle, i, IMAGE_INDEX, (i == button) ? 1 : 0);
			lcd->img_Show(handle, i);
		}
	} else if (action == TOUCH_RELEASED) {
		switch (button) {
			case ibPrepareCustomAccept: // Start printing now!!
				heating_cancelled = true;
				break;
			default:
				break;
		}
	}
	return 0;
}

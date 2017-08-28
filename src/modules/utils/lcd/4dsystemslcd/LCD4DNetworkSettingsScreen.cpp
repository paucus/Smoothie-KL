/*
 * LCD4DNetworkSettingsScreen.cpp
 *
 *  Created on: Nov 16, 2015
 *      Author: eai
 */

#include "LCD4DNetworkSettingsScreen.h"
#include "checksumm.h"
#include "utils.h"
#include "NetworkPublicAccess.h"
#include "lcd_screens.h"
#include <stdio.h>
#include "HttpFacade.h"

LCD4DNetworkSettingsScreen::LCD4DNetworkSettingsScreen() : LCD4DChoiceScreen(LCD4D_NET_SETT_NUMBER_OF_OPTIONS, translate(NETWORK_LABEL)) {

}

LCD4DNetworkSettingsScreen::~LCD4DNetworkSettingsScreen() {
}

int LCD4DNetworkSettingsScreen::on_choice(int num) {

	return 0;
}

const char* LCD4DNetworkSettingsScreen::print_public_data_addr(const char* format_ok, const char* format_fail, uint16_t csb){
	uint8_t* addr_buff;
	addr_buff = get_public_data_ptr<uint8_t>(network_checksum, csb, 0, nullptr);
	if (!addr_buff) {
		strcpy(buff, format_fail);
	} else {
		snprintf(buff, NET_SETTINGS_BUFF_LEN, format_ok, addr_buff[0], addr_buff[1], addr_buff[2], addr_buff[3], csb==get_macaddr_checksum?addr_buff[4]:0, csb==get_macaddr_checksum?addr_buff[5]:0);
		buff[NET_SETTINGS_BUFF_LEN-1] = '\0'; // just in case
	}
	return buff;
}

bool LCD4DNetworkSettingsScreen::is_selectable(int num) {
	return false;
};
const char* LCD4DNetworkSettingsScreen::get_label(int num) {
	// If you make string that could be longer than NET_SETTINGS_BUFF_LEN, remember to enlarge it.
	switch (num) {
	case 0:
		return print_public_data_addr(LCD4DSETT_IP_OK, LCD4DSETT_IP_FAIL, get_ip_checksum);
	case 1:
		return print_public_data_addr(LCD4DSETT_GW_OK, LCD4DSETT_GW_FAIL, get_gw_checksum);
	case 2:
		return print_public_data_addr(LCD4DSETT_DNS_OK, LCD4DSETT_DNS_FAIL, get_dnsaddr_checksum);
	case 3:
		return print_public_data_addr(LCD4DSETT_MAC_OK, LCD4DSETT_MAC_FAIL, get_macaddr_checksum);
	case 4:
		if (HttpFacade::instance.proxy) {
			int written = snprintf(buff, NET_SETTINGS_BUFF_LEN, LCD4DSETT_PROXY_OK, HttpFacade::instance.proxy->get_host(), HttpFacade::instance.proxy->get_port());
			if (written >= NET_SETTINGS_BUFF_LEN){
				// More chars than the number the buffer accepts
				// Write ".." at the end to make it clear that the string is not complete.
				buff[NET_SETTINGS_BUFF_LEN-3] = '.';
				buff[NET_SETTINGS_BUFF_LEN-2] = '.';
				buff[NET_SETTINGS_BUFF_LEN-1] = '\0';
			}
		} else {
			strcpy(buff, LCD4DSETT_PROXY_FAIL);
		}
		return buff;
	}
	return "";
}

LCD4DScreen& LCD4DNetworkSettingsScreen::get_back_screen() {
	return *lcd_screens.settings_screen;
}


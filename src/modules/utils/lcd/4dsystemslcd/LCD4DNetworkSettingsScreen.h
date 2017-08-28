/*
 * LCD4DNetworkSettingsScreen.h
 *
 *  Created on: Nov 16, 2015
 *      Author: eai
 */

#ifndef LCD4DNETWORKSETTINGSSCREEN_H_
#define LCD4DNETWORKSETTINGSSCREEN_H_

#include "LCD4DChoiceScreen.h"

#define LCD4DSETT_IP_OK "IP: %d.%d.%d.%d"
#define LCD4DSETT_IP_FAIL "IP: 0.0.0.0"
#define LCD4DSETT_GW_OK "Gateway: %d.%d.%d.%d"
#define LCD4DSETT_GW_FAIL "Gateway: 0.0.0.0"
#define LCD4DSETT_DNS_OK "DNS: %d.%d.%d.%d"
#define LCD4DSETT_DNS_FAIL "DNS: 0.0.0.0"
#define LCD4DSETT_MAC_OK "MAC: %02x-%02x-%02x-%02x-%02x-%02x"
#define LCD4DSETT_MAC_FAIL "MAC: 00-00-00-00-00-00"
#define LCD4DSETT_PROXY_OK "Proxy: %s:%d"
#define LCD4DSETT_PROXY_FAIL "Proxy: -"
#define LCD4D_NET_SETT_NUMBER_OF_OPTIONS 5

#define NET_SETTINGS_BUFF_LEN 35

class LCD4DNetworkSettingsScreen: public LCD4DChoiceScreen {
public:
	LCD4DNetworkSettingsScreen();
	virtual ~LCD4DNetworkSettingsScreen();
protected:
	int on_choice(int num);
	bool is_selectable(int num);
	const char* get_label(int num);
	LCD4DScreen& get_back_screen();
private:
	const char* print_public_data_addr(const char* format_ok, const char* format_fail, uint16_t csb);
	char buff[NET_SETTINGS_BUFF_LEN];
};

#endif /* LCD4DNETWORKSETTINGSSCREEN_H_ */

/*
 * HttpProxyMod.cpp
 *
 *  Created on: Nov 13, 2015
 *      Author: eai
 */

#include "HttpProxyMod.h"
#include "HttpFacade.h"
#include "Config.h"
#include "ConfigValue.h"
#include "checksumm.h"
#include "NetworkPublicAccess.h"
#include "Kernel.h"
#include "StreamOutput.h"
#include <string>
#include <cstdlib>
using namespace std;
#include "utils.h"

#define proxy_checksum        CHECKSUM("proxy")
#define enable_checksum       CHECKSUM("enable")
#define host_checksum         CHECKSUM("host")
#define port_checksum         CHECKSUM("port")

HttpProxyMod::HttpProxyMod() {
}

HttpProxyMod::~HttpProxyMod() {
}

void HttpProxyMod::on_module_loaded() {
	if (!THEKERNEL->config->value(network_checksum, proxy_checksum, enable_checksum)->by_default(true)->as_bool()) {
		delete this;
		return;
	}

	string host = THEKERNEL->config->value(network_checksum, proxy_checksum, enable_checksum)->by_default("")->as_string();
	uint16_t port = THEKERNEL->config->value(network_checksum, proxy_checksum, enable_checksum)->by_default(0)->as_int();

	if (host.length() > 0 && port > 0) {
		HttpFacade::instance.proxy = new HttpProxy(host.c_str(), port);
	}

	this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
}

void HttpProxyMod::on_console_line_received(void * args) {
	SerialMessage* msg = static_cast<SerialMessage *>(args);
	string parameters = msg->message;
	string possible_command = shift_parameter(parameters);
	if (possible_command == "help") {
		msg->stream->printf("http-proxy (host port|none) - Use the given host+port combination as main http proxy, or none if no proxy must be used\n");
	} else if (possible_command == "http-proxy") {
		string p1 = shift_parameter(parameters);
		if (p1 == "") {
			if (!HttpFacade::instance.proxy) {
				msg->stream->printf("Proxy=none\n");
			} else {
				msg->stream->printf("Proxy=%s:%u\n", HttpFacade::instance.proxy->get_host(), HttpFacade::instance.proxy->get_port());
			}
			return;
		}
		string p2 = parameters;
		unsigned int port = strtoul(p2.c_str(), NULL, 10);
		if (HttpFacade::instance.proxy) {
			delete HttpFacade::instance.proxy;
			HttpFacade::instance.proxy = nullptr;
		}
		if (p1 != "none" && port > 0) {
			// Proxy specified
			HttpFacade::instance.proxy = new HttpProxy(p1.c_str(), port);
			msg->stream->printf("Proxy set to %s:%u\n", p1.c_str(), port);
		} else if (p1 == "none" && port == 0) {
			// proxy = none
			msg->stream->printf("Proxy set to none\n");
		} else {
			msg->stream->printf("Wrong parameters\n");
		}
	}
}


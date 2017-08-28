/*
 * HttpProxyMod.h
 *
 *  Created on: Nov 13, 2015
 *      Author: eai
 */

#ifndef HTTPPROXYMOD_H_
#define HTTPPROXYMOD_H_

#include "Module.h"

// This module is responsible for handling the requests to connect to a proxy, and load the proxy
// configuration when the printer boots.
class HttpProxyMod: public Module {
public:
	HttpProxyMod();
	virtual ~HttpProxyMod();

	void on_module_loaded();
	void on_console_line_received(void *);
};

#endif /* HTTPPROXYMOD_H_ */

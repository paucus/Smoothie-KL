/*
 * MiscModule.h
 *
 *  Created on: Mar 2, 2016
 *      Author: eai
 */

#ifndef MISCMODULE_H_
#define MISCMODULE_H_

#include "Module.h"

// This module groups functions that are small and are not part of other
// modules.
class MiscModule: public Module {
public:
	MiscModule();
	virtual ~MiscModule();

	void on_module_loaded();
	void on_print_status_change(void *);
};

#endif /* MISCMODULE_H_ */

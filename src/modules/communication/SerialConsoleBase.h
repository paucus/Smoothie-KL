/*
 * SerialConsoleBase.h
 *
 *  Created on: Sep 16, 2015
 *      Author: eai
 */

#ifndef SERIALCONSOLEBASE_H_
#define SERIALCONSOLEBASE_H_

#include "libs/Module.h"
#include "Serial.h" // mbed.h lib
#include "libs/Kernel.h"
#include <vector>
#include <string>
using std::string;
#include "libs/RingBuffer.h"
#include "libs/StreamOutput.h"

class SerialConsoleBase: public Module, public StreamOutput {
public:
	SerialConsoleBase();
	virtual ~SerialConsoleBase();
	virtual size_t buffer_size() = 0;
	virtual void buffer_pop_front(char & object) = 0;
	virtual void buffer_delete_tail() = 0;
	virtual void serial_baud(int baudrate) = 0;
};

#endif /* SERIALCONSOLEBASE_H_ */

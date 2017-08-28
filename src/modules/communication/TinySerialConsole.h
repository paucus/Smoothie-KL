/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TINY_SERIAL_CONSOLE_H
#define TINY_SERIAL_CONSOLE_H

#include "libs/Module.h"
#include "Serial.h" // mbed.h lib
#include "libs/Kernel.h"
#include <vector>
#include <string>
using std::string;
#include "libs/RingBuffer.h"
#include "libs/StreamOutput.h"


#define baud_rate_setting_checksum CHECKSUM("baud_rate")

// BEGIN MODIF memory_usage
#include "SerialConsoleBase.h"
// This class is a copy of SerialConsole, but with less buffer.
// My plan is to use it but trying not to modify the original one.
class TinySerialConsole : public SerialConsoleBase {
// END MODIF memory_usage
    public:
        TinySerialConsole( PinName rx_pin, PinName tx_pin, int baud_rate );

        // BEGIN MODIF second_serial
        // Made this method virtual, so that we can avoid the execution of code taken from this serial
        virtual void on_module_loaded();
        virtual void on_serial_char_received();
        // END MODIF second_serial
        void on_main_loop(void * argument);
        bool has_char(char letter);

        int _putc(int c);
        int _getc(void);
        int puts(const char*);

        //string receive_buffer;                 // Received chars are stored here until a newline character is received
        //vector<std::string> received_lines;    // Received lines are stored here until they are requested
        RingBuffer<char,32> buffer;             // Receive buffer
        mbed::Serial* serial;

        // BEGIN MODIF memory_usage
        size_t buffer_size() {return this->buffer.size();};
        void buffer_pop_front(char & object){ this->buffer.pop_front(object);};
        void buffer_delete_tail(){ this->buffer.delete_tail();};
        void serial_baud(int baudrate){ this->serial->baud(baudrate);};
        // END MODIF memory_usage
};

#endif

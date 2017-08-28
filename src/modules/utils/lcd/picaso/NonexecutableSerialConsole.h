#ifndef NONEXECUTABLE_SERIAL_CONSOLE
#define NONEXECUTABLE_SERIAL_CONSOLE

#include "TinySerialConsole.h"

class NonexecutableSerialConsole : public TinySerialConsole {
public:
    NonexecutableSerialConsole( PinName tx_pin, PinName rx_pin, int baud_rate );
    virtual void on_module_loaded();
    virtual void on_serial_char_received();
};

#endif // NONEXECUTABLE_SERIAL_CONSOLE


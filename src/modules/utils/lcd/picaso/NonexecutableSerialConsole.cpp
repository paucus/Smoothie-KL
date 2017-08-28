#include "NonexecutableSerialConsole.h"

NonexecutableSerialConsole::NonexecutableSerialConsole( PinName tx_pin, PinName rx_pin, int baud_rate ) : TinySerialConsole( tx_pin, rx_pin, baud_rate ) {
}

// Called when the module has just been loaded
void NonexecutableSerialConsole::on_module_loaded() {
    // don't call the parent method!! We want to prevent it from registering on the main loop

    // We want to be called every time a new char is received
    this->serial->attach((NonexecutableSerialConsole*)this, &NonexecutableSerialConsole::on_serial_char_received, mbed::Serial::RxIrq);

    // We only call the command dispatcher in the main loop, nowhere else
    // REMOVED
    //this->register_for_event(ON_MAIN_LOOP);

    // Add to the pack of streams kernel can call to, for example for broadcasting
    // REMOVED
    //THEKERNEL->streams->append_stream(this);
}


void NonexecutableSerialConsole::on_serial_char_received(){
    while(this->serial->readable()){
        char received = this->serial->getc();
        // convert CR to NL (for host OSs that don't send NL)
        //if( received == '\r' ){ received = '\n'; }
        this->buffer.push_back(received);
    }
}


#ifndef SERIAL_CONSOLE_STREAM
#define SERIAL_CONSOLE_STREAM

#include "SerialConsoleBase.h"
#include "VirtualPort.h"

// This class is an adapter class from SerialConsole to the arduino VirtualPort class.
class SerialConsoleStream: public VirtualPort {
public:
	SerialConsoleStream(SerialConsoleBase* con);
	virtual ~SerialConsoleStream();
	int available();
	int read();
	int peek();
	void flush();
	size_t write(uint8_t v);

	void set_baudrate(int new_baudrate);
private:
	SerialConsoleBase* con;
};

#endif // SERIAL_CONSOLE_STREAM

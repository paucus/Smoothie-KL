#include "SerialConsoleStream.h"

#include "Kernel.h"
#include "StreamOutput.h"
#include "StreamOutputPool.h"

SerialConsoleStream::SerialConsoleStream(SerialConsoleBase* con) {
	this->con = con;
}

SerialConsoleStream::~SerialConsoleStream() {
}

int SerialConsoleStream::available() {
	return con->buffer_size();
}

int SerialConsoleStream::read() {
	char res = 0;
	con->buffer_pop_front(res);
	return res;
}

int SerialConsoleStream::peek() {
	// not used
	return this->read();
}

void SerialConsoleStream::flush() {

}

size_t SerialConsoleStream::write(uint8_t v) {
	return con->_putc((int) v);
}

void SerialConsoleStream::set_baudrate(int new_baudrate) {
	this->con->serial_baud(new_baudrate);
}

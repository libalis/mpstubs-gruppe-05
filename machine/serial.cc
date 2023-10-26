#include "machine/serial.h"

Serial::Serial(ComPort port, BaudRate baud_rate, DataBits data_bits, StopBits stop_bits, Parity parity) : port(port) {
	// TODO: Implement (if you want, optional exercise)
	(void) baud_rate;
	(void) data_bits;
	(void) stop_bits;
	(void) parity;
}

void Serial::writeReg(RegisterIndex reg, char out) {
	// TODO: Implement (if you want, optional exercise)
	(void) reg;
	(void) out;
}

char Serial::readReg(RegisterIndex reg) {
	// TODO: Implement (if you want, optional exercise)
	(void) reg;
	return '\0';
}

int Serial::write(char out, bool blocking) {
	// TODO: Implement (if you want, optional exercise)
	(void) out;
	(void) blocking;
	return 0;
}

int Serial::read(bool blocking) {
	// TODO: Implement (if you want, optional exercise)
	(void) blocking;
	return 0;
}


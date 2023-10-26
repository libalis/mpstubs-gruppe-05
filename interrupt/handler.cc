#include "interrupt/handler.h"

extern "C" void interrupt_handler(Core::Interrupt::Vector vector, InterruptContext *context) {
	(void) vector;
	(void) context;

}

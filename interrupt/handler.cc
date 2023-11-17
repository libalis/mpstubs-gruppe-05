#include "interrupt/handler.h"
#include "interrupt/plugbox.h"
#include "machine/lapic.h"

extern "C" void interrupt_handler(Core::Interrupt::Vector vector, InterruptContext *context) {
	Plugbox::report(vector)->trigger();
	LAPIC::endOfInterrupt();
}

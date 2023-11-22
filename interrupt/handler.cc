#include "interrupt/handler.h"
#include "debug/output.h"
#include "interrupt/plugbox.h"
#include "machine/core.h"
#include "machine/lapic.h"

extern "C" void interrupt_handler(Core::Interrupt::Vector vector, InterruptContext *context) {
	if (vector < Core::Interrupt::EXCEPTIONS) {
		DBG << "error_code: " << context->error_code << endl;
	}
	Plugbox::report(vector)->trigger();
	LAPIC::endOfInterrupt();
}

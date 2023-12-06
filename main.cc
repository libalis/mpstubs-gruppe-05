#include "boot/startup_ap.h"
#include "debug/output.h"
#include "device/keyboard.h"
#include "interrupt/gatequeue.h"
#include "machine/core.h"
#include "machine/ioapic.h"
#include "machine/lapic.h"
#include "machine/ps2controller.h"
#include "sync/ticketlock.h"
#include "user/app1/appl.h"
#include "utils/string.h"

TextStream dout[Core::MAX]{
	{0, TextMode::COLUMNS/2, 18, 21},
	{TextMode::COLUMNS/2, TextMode::COLUMNS, 18, 21},
	{0, TextMode::COLUMNS/2, 22, 25},
	{TextMode::COLUMNS/2, TextMode::COLUMNS, 22, 25},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
	{0, 0, 0, 0},
};
TextStream kout{0, TextMode::COLUMNS, 0, 17, true};

Keyboard keyboard{};

GateQueue gatequeue{};

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	kout.reset();
	DBG.reset();

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	IOAPIC::init();

	PS2Controller::init();
	PS2Controller::drainBuffer();

	keyboard.plugin();

	// Start application processors
	ApplicationProcessor::boot();

	DBG << "CPU " << Core::getID() << " ready" << endl;

	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Core::Interrupt::enable();

	Application{}.action();

	return 0;
}

// Main function for application processors
extern "C" int main_ap() {
	DBG.reset();

	DBG << "CPU " << Core::getID() << " ready" << endl;

	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Core::Interrupt::enable();

	Application{}.action();

	return 0;
}

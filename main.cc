#include "boot/startup_ap.h"
#include "debug/output.h"
#include "device/keyboard.h"
#include "device/watch.h"
#include "interrupt/guard.h"
#include "machine/core.h"
#include "machine/ioapic.h"
#include "machine/lapic.h"
#include "thread/assassin.h"
#include "thread/scheduler.h"
#include "user/app1/appl.h"

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

Application app[Core::MAX + 1]{};

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	kout.reset();
	DBG.reset();

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	IOAPIC::init();

	keyboard.plugin();
	assassin.hire();

	for (unsigned int i = 0; i < Core::MAX + 1; i++)
		Scheduler::ready(&app[i]);

	watch[Core::getID()].windup(5000000);

	// Start application processors
	ApplicationProcessor::boot();

	watch[Core::getID()].activate();

	Core::Interrupt::enable();

	DBG << "CPU " << Core::getID() << " ready" << endl;

	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Guard::enter();
	Scheduler::schedule();

	return 0;
}

// Main function for application processors
extern "C" int main_ap() {
	watch[Core::getID()].activate();

	Core::Interrupt::enable();

	DBG.reset();

	DBG << "CPU " << Core::getID() << " ready" << endl;

	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Guard::enter();
	Scheduler::schedule();

	return 0;
}

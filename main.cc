#include "boot/startup_ap.h"
#include "debug/output.h"
#include "device/watch.h"
#include "interrupt/guard.h"
#include "machine/core.h"
#include "machine/ioapic.h"
#include "machine/lapic.h"
#include "syscall/guarded_keyboard.h"
#include "thread/assassin.h"
#include "thread/scheduler.h"
#include "thread/wakeup.h"
#include "user/app1/appl.h"
#include "user/app2/kappl.h"

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
KeyboardApplication kapp{};

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	kout.reset();
	DBG.reset();

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	IOAPIC::init();

	assassin.hire();
	guardedkeyboard.plugin();
	wakeup.activate();

	for (unsigned int i = 0; i < Core::MAX + 1; i++)
		Scheduler::ready(&app[i]);

	Scheduler::ready(&kapp);

	assert(watch.windup(1000));

	// Start application processors
	ApplicationProcessor::boot();

	watch.activate();

	DBG << "CPU " << Core::getID() << " ready" << endl;

	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Guard::enter();
	Core::Interrupt::enable();
	Scheduler::schedule();

	return 0;
}

// Main function for application processors
extern "C" int main_ap() {
	watch.activate();

	DBG.reset();

	DBG << "CPU " << Core::getID() << " ready" << endl;

	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Guard::enter();
	Core::Interrupt::enable();
	Scheduler::schedule();

	return 0;
}

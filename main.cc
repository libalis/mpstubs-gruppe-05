#include "boot/startup_ap.h"
#include "debug/output.h"
#include "device/keyboard.h"
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
TextStream kbout{0, TextMode::COLUMNS, 0, 1};

Ticketlock ticketlock{};

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	TextStream aout{0, TextMode::COLUMNS, 0, TextMode::ROWS};
	aout.reset();
	aout.setPos(0, 17);
	aout.print("CPU 0 ready", strlen("CPU 0 ready"));
	aout.setPos(TextMode::COLUMNS/2, 17);
	aout.print("CPU 1 ready", strlen("CPU 1 ready"),
		TextMode::Attribute(static_cast<TextMode::Color>(TextMode::WHITE - 1)));
	aout.setPos(0, 21);
	aout.print("CPU 2 ready", strlen("CPU 2 ready"),
		TextMode::Attribute(static_cast<TextMode::Color>(TextMode::WHITE - 2)));
	aout.setPos(TextMode::COLUMNS/2, 21);
	aout.print("CPU 3 ready", strlen("CPU 3 ready"),
		TextMode::Attribute(static_cast<TextMode::Color>(TextMode::WHITE - 3)));

	kout.reset();

	// Start application processors
	ApplicationProcessor::boot();

	PS2Controller::init();
	PS2Controller::drainBuffer();
	Keyboard{}.plugin();

	Application{}.action();

	return 0;
}

// Main function for application processors
extern "C" int main_ap() {
	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	Application{}.action();

	return 0;
}

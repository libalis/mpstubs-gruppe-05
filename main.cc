#include "boot/startup_ap.h"
#include "machine/lapic.h"
#include "debug/output.h"
#include "user/app1/appl.h"
#include "user/app2/kappl.h"
#include "utils/string.h"

TextStream kout(0, TextMode::COLUMNS, 0, 17, true);
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

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	// Start application processors
	ApplicationProcessor::boot();

	TextStream aout{0, TextMode::COLUMNS, 0, TextMode::ROWS};
	aout.reset();
	aout.setPos(0, 17);
	aout.print("CPU 0 ready", strlen("CPU 0 ready"));
	aout.setPos(TextMode::COLUMNS/2, 17);
	aout.print("CPU 1 ready", strlen("CPU 1 ready"), TextMode::Attribute(TextMode::YELLOW));
	aout.setPos(0, 21);
	aout.print("CPU 2 ready", strlen("CPU 2 ready"), TextMode::Attribute(TextMode::MAGENTA));
	aout.setPos(TextMode::COLUMNS/2, 21);
	aout.print("CPU 3 ready", strlen("CPU 3 ready"), TextMode::Attribute(TextMode::RED));

	kout.reset();
	Application().action();
	KeyboardApplication().action();

	return 0;
}

// Main function for application processors
extern "C" int main_ap() {
	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	return 0;
}

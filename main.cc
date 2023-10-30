#include "boot/startup_ap.h"
#include "machine/lapic.h"
#include "debug/output.h"
#include "user/app1/appl.h"
#include "user/app2/kappl.h"

TextStream kout(0, TextMode::COLUMNS, 0, TextMode::ROWS-Core::MAX/2*3, true);
TextStream dout[Core::MAX]{
	{0, 40, 13, 16},
	{40, 80, 13, 16},
	{0, 40, 16, 19},
	{40, 80, 16, 19},
	{0, 40, 19, 22},
	{40, 80, 19, 22},
	{0, 40, 22, 25},
	{40, 80, 22, 25},
};

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	// Start application processors
	ApplicationProcessor::boot();

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

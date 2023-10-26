#include "boot/startup_ap.h"
#include "machine/lapic.h"
#include "debug/output.h"

const char * os_name = "MP" "StuBS";

// Main function (the bootstrap processor starts here)
extern "C" int main() {

	unsigned int num_cpus = Core::count();
	DBG_VERBOSE << "Number of CPUs: " << num_cpus << endl;

	// Start application processors
	ApplicationProcessor::boot();

	return 0;
}

// Main function  for application processors
extern "C" int main_ap() {
	DBG_VERBOSE << "CPU core " << static_cast<int>(Core::getID())
	            << " / LAPIC " << static_cast<int>(LAPIC::getID()) << " in main_ap()" << endl;

	return 0;
}

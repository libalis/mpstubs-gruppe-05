#include "startup.h"

#include "types.h"
#include "compiler/libc.h"
#include "debug/output.h"
#include "debug/kernelpanic.h"
#include "interrupt/handler.h"
#include "machine/acpi.h"
#include "machine/apic.h"
#include "machine/core.h"
#include "machine/idt.h"
#include "machine/pic.h"

/*! \brief The first processor is the Bootstrap Processor (BSP)
 */
static bool is_bootstrap_processor = true;

extern "C" [[noreturn]] void kernel_init() {
	if (is_bootstrap_processor) {
		is_bootstrap_processor = false;
		/* Setup and load Interrupt Descriptor Table (IDT)
		 *
		 * On the first call to \ref kernel_init(), we have to assign the
		 * addresses of the entry functions for each interrupt
		 * (called 'interrupt_entry_VECTOR', defined in `interrupt/handler.asm`)
		 * to the IDT. These entry functions save the context and call the C++
		 * \ref interrupt_handler(). As the \ref IDT is used by all CPUs,
		 * it is sufficient to do this initialization on only the BSP (first core).
		 */
		for (int i = 0; i < Core::Interrupt::VECTORS ; i++) {
			IDT::handle(i, interrupt_entry[i]);
		}
		IDT::load();

		// Initialize PICs
		PIC::initialize();

		// Call global constructors
		CSU::initializer();

		// Initialize ACPI
		if (!ACPI::init()) {
			DBG_VERBOSE << "No ACPI!";
			Core::die();
		}
		// Initialize APIC (using ACPI)
		if (!APIC::init()) {
			DBG_VERBOSE << "APIC Initialization failed";
			Core::die();
		}

		// Initialize the Bootstrap Processor
		Core::init();

		// Go to main function
		main();

		// Exit CPU
		DBG_VERBOSE << "CPU core " << Core::getID() << " (BSP) shutdown." << endl;
		Core::exit();
	} else {
		// Load Interrupt Descriptor Table (IDT)
		IDT::load();

		// Initialize this application processor
		Core::init();

		// And call the AP main
		main_ap();

		// Exit CPU
		DBG_VERBOSE << "CPU core " << Core::getID() << " (AP) shutdown." << endl;
		Core::exit();
	}

	// Only on last core
	if (Core::countOnline() == 1) {
		// Call global destructors
		CSU::finalizer();
	}

	// wait forever
	while (true) {
		Core::die();
	}
}

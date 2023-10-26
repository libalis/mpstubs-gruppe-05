#include "startup_ap.h"
#include "utils/string.h"
#include "utils/size.h"
#include "debug/output.h"
#include "debug/assert.h"
#include "machine/lapic.h"
#include "machine/core_interrupt.h"
#include "machine/gdt.h"
#include "machine/pit.h"

namespace ApplicationProcessor {

// Make sure that the RELOCATED_SETUP is in low memory (< 1 MiB)
static_assert((RELOCATED_SETUP & ~0x000ff000) == 0, "Not a valid 1 MB address for RELOCATED_SETUP!");

/*! \brief Temporary Global Descriptor Table
 *
 * Blue print, to be copied into real mode code
 */
constexpr GDT::SegmentDescriptor ap_gdt[] = {
	// nullptr-Deskriptor
	{},

	// code segment
	{	/* base  = */ 0x0,
		/* limit = */ 0xFFFFFFFF,
		/* code  = */ true,
		/* ring  = */ 0,
		/* size  = */ GDT::SIZE_32BIT  },

	// data segment
	{	/* base  = */ 0x0,
		/* limit = */ 0xFFFFFFFF,
		/* code  = */ false,
		/* ring  = */ 0,
		/* size  = */ GDT::SIZE_32BIT  },
};

void relocateSetupCode() {
	// Relocated setup code
	memcpy(reinterpret_cast<void*>(RELOCATED_SETUP), &___SETUP_AP_START__, &___SETUP_AP_END__ - &___SETUP_AP_START__);

	// Adjust GDT:
	// Calculate offset for real mode GDT and GDT descriptor
	uintptr_t ap_gdt_offset = reinterpret_cast<uintptr_t>(&setup_ap_gdt) -
	                          reinterpret_cast<uintptr_t>(&___SETUP_AP_START__);
	uintptr_t ap_gdtd_offset = reinterpret_cast<uintptr_t>(&setup_ap_gdtd) -
	                           reinterpret_cast<uintptr_t>(&___SETUP_AP_START__);

	// Copy blue print of real mode GDT to the relocated memory
	void * relocated_ap_gdt = reinterpret_cast<void*>(RELOCATED_SETUP + ap_gdt_offset);
	memcpy(relocated_ap_gdt, &ap_gdt, sizeof(ap_gdt));

	// Calculate GDT descriptor for relocated address
	GDT::Pointer * relocated_ap_gdtd = reinterpret_cast<GDT::Pointer*>(RELOCATED_SETUP + ap_gdtd_offset);
	relocated_ap_gdtd->set(relocated_ap_gdt, size(ap_gdt));
}

void boot(void) {
	assert(!Core::Interrupt::isEnabled() && "Interrupts should not be enabled before APs have booted!");

	// Relocate setup code
	relocateSetupCode();

	// Calculate Init-IPI vector based on address of relocated setup_ap()
	uint8_t vector = RELOCATED_SETUP >> 12;

	// Send Init-IPI to all APs
	LAPIC::IPI::sendInit();

	// wait at least 10ms
	PIT::delay(10000);

	// Send Startup-IPI twice
	DBG_VERBOSE << "Sending STARTUP IPI #1" << endl;
	LAPIC::IPI::sendStartup(vector);
	// wait at least 200us
	PIT::delay(200);

	DBG_VERBOSE << "Sending STARTUP IPI #2" << endl;
	LAPIC::IPI::sendStartup(vector);
}
}  // namespace ApplicationProcessor

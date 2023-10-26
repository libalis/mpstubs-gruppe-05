#include "machine/apic.h"
#include "machine/acpi.h"
#include "machine/core.h"
#include "machine/cmos.h"
#include "machine/ioport.h"
#include "machine/lapic.h"
#include "machine/lapic_registers.h"
#include "utils/string.h"
#include "debug/assert.h"
#include "debug/output.h"

namespace APIC {

static struct {
	uint32_t id;
	uintptr_t address;
	uint32_t interrupt_base;
} ioapic;

static uint8_t slot_map[16];

static uint8_t lapic_id[Core::MAX];
static unsigned lapics = 0;

bool init() {
	// get Multiple APIC Definition Table (MADT) from ACPI
	ACPI::MADT *madt = static_cast<ACPI::MADT*>(ACPI::get('A', 'P', 'I', 'C'));
	if(madt == 0) {
		DBG_VERBOSE << "ERROR: no MADT found in ACPI" << endl;
		return false;
	}

	// read the local APIC address
	LAPIC::base_address = static_cast<uintptr_t>(madt->local_apic_address);
	DBG_VERBOSE << "LAPIC Address "
	            << reinterpret_cast<void*>(static_cast<uintptr_t>(madt->local_apic_address)) << endl;

	// PC/AT compatibility mode
	if (madt->flags_pcat_compat != 0) {
		// The APIC operating mode is set to compatible PIC mode - we have to change it.
		DBG_VERBOSE << "PIC comp mode, disabling PICs." << endl;

		// Select Interrupt Mode Control Register (IMCR)
		// (this register will only exist if hardware supports the PIC mode)
		IOPort reg(0x22);
		reg.outb(0x70);
		// disable PIC mode, use APIC
		IOPort imcr(0x23);
		imcr.outb(0x01);
	}

	// Set default mapping of external interrupt slots (might be overwritten below)
	for (unsigned i = 0; i < sizeof(slot_map)/sizeof(slot_map[0]); i++) {
		slot_map[i] = i;
	}

	// Initialize invalid lapic_ids
	for (unsigned i = 0; i < Core::MAX; i++) {
		lapic_id[i] = INVALID_ID;
	}

	// reset numbers, store apic data into arrays
	for (ACPI::SubHeader *mads = madt->first(); mads < madt->end(); mads = mads->next()) {
		switch(mads->type) {
			case ACPI::MADS::Type_LAPIC:
			  {
				ACPI::MADS::LAPIC* mads_lapic = static_cast<ACPI::MADS::LAPIC*>(mads);
				if (mads_lapic->flags_enabled == 0) {
					DBG_VERBOSE << "Detected disabled LAPIC with ID " << static_cast<unsigned>(mads_lapic->apic_id) << endl;
				} else if (lapics >= Core::MAX) {
					DBG_VERBOSE << "Got more LAPICs than Core::MAX" << endl;
				} else if (mads_lapic->apic_id == INVALID_ID) {
					DBG_VERBOSE << "Got invalid APIC ID" << endl;
				} else {
					DBG_VERBOSE << "Detected LAPIC with ID " << static_cast<unsigned>(mads_lapic->apic_id) << endl;
					lapic_id[lapics++] = mads_lapic->apic_id;
				}
				break;
			  }
			case ACPI::MADS::Type_IOAPIC:
			  {
				ACPI::MADS::IOAPIC* mads_ioapic = static_cast<ACPI::MADS::IOAPIC*>(mads);
				DBG_VERBOSE << "Detected IO APIC with ID " << static_cast<unsigned>(mads_ioapic->ioapic_id) << " / Base "
				            << reinterpret_cast<void*>(static_cast<uintptr_t>(mads_ioapic->global_system_interrupt_base))
				            << endl;
				if (mads_ioapic->global_system_interrupt_base > 23) {
					DBG_VERBOSE << "Ignoring IOAPIC since we currently only support one." << endl;
				} else {
					ioapic.id = mads_ioapic->ioapic_id;
					ioapic.address = static_cast<uintptr_t>(mads_ioapic->ioapic_address);
					ioapic.interrupt_base = mads_ioapic->global_system_interrupt_base;
				}
				break;
			  }
			case ACPI::MADS::Type_Interrupt_Source_Override:
			  {
				ACPI::MADS::Interrupt_Source_Override* mads_iso = static_cast<ACPI::MADS::Interrupt_Source_Override*>(mads);
				if (mads_iso->bus == 0) {
					DBG_VERBOSE << "Overriding Interrupt Source " << static_cast<unsigned>(mads_iso->source)
					            << " with " <<  mads_iso->global_system_interrupt << endl;
					if (mads_iso->source < sizeof(slot_map)/sizeof(slot_map[0])) {
						slot_map[mads_iso->source] = mads_iso->global_system_interrupt;
					}
				} else {
					DBG_VERBOSE << "Override for bus " << mads_iso->bus << " != ISA. Does not conform to ACPI." << endl;
				}
				break;
			  }
			case ACPI::MADS::Type_LAPIC_Address_Override:
			  {
				ACPI::MADS::LAPIC_Address_Override* mads_lao = static_cast<ACPI::MADS::LAPIC_Address_Override*>(mads);
				LAPIC::base_address = static_cast<uintptr_t>(mads_lao->lapic_address_low);
				DBG_VERBOSE << "Overriding LAPIC address with "
				            << reinterpret_cast<void*>(static_cast<uintptr_t>(mads_lao->lapic_address)) << endl;
				break;
			  }
		}
	}
	return true;
}

uint8_t getIOAPICSlot(APIC::Device device) {
	return slot_map[device];
}

uintptr_t getIOAPICAddress() {
	return ioapic.address;
}

uint8_t getIOAPICID() {
	return ioapic.id;
}

uint8_t getLogicalAPICID(uint8_t core) {
	return core < Core::MAX ? (1 << core) : 0;
}

uint8_t getLAPICID(uint8_t core) {
	assert(core < Core::MAX);
	return lapic_id[core];
}

}  // namespace APIC

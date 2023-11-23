#include "ioapic.h"

#include "machine/apic.h"
#include "machine/core.h"
#include "debug/assert.h"

namespace IOAPIC {
/*! \brief IOAPIC registers memory mapped into the CPU's address space.
 *
 *  Access to the actual IOAPIC registers can be obtained by performing the following steps:
 *  1. Write the number of the IOAPIC register to the address stored in `IOREGSEL_REG`
 *  2. Read the value from / write the value to the address referred to by `IOWIN_REG`.
 *
 *  \see [IO-APIC manual](intel_ioapic.pdf#page=8)
 */
volatile Index *IOREGSEL_REG = reinterpret_cast<volatile Index*>(0xfec00000);
/// \copydoc IOREGSEL_REG
volatile Register *IOWIN_REG = reinterpret_cast<volatile Register*>(0xfec00010);

// IOAPIC manual, p. 8
const Index IOAPICID_IDX = 0x00;
const Index IOREDTBL_IDX = 0x10;
const Index IOREDTBL_ENTRY_SIZE = 0x02;

const uint8_t slot_max = 24;

RedirectionTableEntry readEntry(Index slot) {
    *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
    Register low = *IOWIN_REG;
    *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
    Register high = *IOWIN_REG;
    return RedirectionTableEntry{low, high};
}

void writeEntry(Index slot, RedirectionTableEntry entry) {
    *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
    *IOWIN_REG = entry.value_low;
    *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
    *IOWIN_REG = entry.value_high;
}

void init() {
    for (uint8_t slot = 0; slot < slot_max; slot++) {
        RedirectionTableEntry entry = readEntry(slot);
        entry.destination = (1 << Core::count()) - 1;
        entry.interrupt_mask = MASKED;
        entry.trigger_mode = EDGE;
        entry.polarity = HIGH;
        entry.destination_mode = LOGICAL;
        entry.delivery_mode = LOWEST_PRIORITY;
        entry.vector = Core::Interrupt::PANIC;
        writeEntry(slot, entry);
    }
    *IOREGSEL_REG = IOAPICID_IDX;
    Identification IOAPICID{*IOWIN_REG};
    IOAPICID.id = APIC::getIOAPICID();
    *IOWIN_REG = IOAPICID.value;
}

void config(uint8_t slot, Core::Interrupt::Vector vector, TriggerMode trigger_mode, Polarity polarity) {
    assert(slot < slot_max);
    RedirectionTableEntry entry = readEntry(slot);
    entry.vector = vector;
    entry.trigger_mode = trigger_mode;
    entry.polarity = polarity;
    writeEntry(slot, entry);
}

void allow(uint8_t slot) {
    assert(slot < slot_max);
    RedirectionTableEntry entry = readEntry(slot);
    entry.interrupt_mask = UNMASKED;
    writeEntry(slot, entry);
}

void forbid(uint8_t slot) {
    assert(slot < slot_max);
    RedirectionTableEntry entry = readEntry(slot);
    entry.interrupt_mask = MASKED;
    writeEntry(slot, entry);
}

bool status(uint8_t slot) {
    assert(slot < slot_max);
    return readEntry(slot).interrupt_mask == UNMASKED;
}

}  // namespace IOAPIC

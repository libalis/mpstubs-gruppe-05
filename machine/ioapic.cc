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

void init() {
    for (uint8_t slot = 0; slot < slot_max; slot++) {
        RedirectionTableEntry entry{0, 0};
        uint8_t destination_field = 0;
        for (unsigned int i = 0; i < Core::count(); i++)
            destination_field |= (1 << i);
        entry.destination = destination_field;
        entry.interrupt_mask = MASKED;
        entry.trigger_mode = EDGE;
        entry.polarity = HIGH;
        entry.destination_mode = LOGICAL;
        entry.delivery_mode = LOWEST_PRIORITY;
        entry.vector = Core::Interrupt::PANIC;
        *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
        *IOWIN_REG = entry.value_low;
        *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
        *IOWIN_REG = entry.value_high;
    }
    *IOREGSEL_REG = IOAPICID_IDX;
    uint8_t id = ~APIC::getIOAPICID();
    for (unsigned int i = 4; i < 8; i++)
            id &= ~(1 << i);
    uint32_t long_id = id << 24;
    long_id = ~long_id;
    *IOWIN_REG &= long_id;
}

void config(uint8_t slot, Core::Interrupt::Vector vector, TriggerMode trigger_mode, Polarity polarity) {
    assert(slot < slot_max);
    *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
    Register low = *IOWIN_REG;
    *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
    Register high = *IOWIN_REG;
    RedirectionTableEntry entry{low, high};
    entry.vector = vector;
    entry.trigger_mode = trigger_mode;
    entry.polarity = polarity;
    *IOWIN_REG = entry.value_high;
    *IOREGSEL_REG -= IOREDTBL_ENTRY_SIZE / 2;
    *IOWIN_REG = entry.value_low;
}

void allow(uint8_t slot) {
    assert(slot < slot_max);
    *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
    Register low = *IOWIN_REG;
    *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
    Register high = *IOWIN_REG;
    RedirectionTableEntry entry{low, high};
    entry.interrupt_mask = UNMASKED;
    *IOWIN_REG = entry.value_high;
    *IOREGSEL_REG -= IOREDTBL_ENTRY_SIZE / 2;
    *IOWIN_REG = entry.value_low;
    Core::Interrupt::enable();
}

void forbid(uint8_t slot) {
    assert(slot < slot_max);
    *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
    Register low = *IOWIN_REG;
    *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
    Register high = *IOWIN_REG;
    RedirectionTableEntry entry{low, high};
    entry.interrupt_mask = MASKED;
    *IOWIN_REG = entry.value_high;
    *IOREGSEL_REG -= IOREDTBL_ENTRY_SIZE / 2;
    *IOWIN_REG = entry.value_low;
}

bool status(uint8_t slot) {
    assert(slot < slot_max);
    *IOREGSEL_REG = IOREDTBL_IDX + slot * IOREDTBL_ENTRY_SIZE;
    Register low = *IOWIN_REG;
    *IOREGSEL_REG += IOREDTBL_ENTRY_SIZE / 2;
    Register high = *IOWIN_REG;
    RedirectionTableEntry entry{low, high};
    return entry.interrupt_mask == UNMASKED;
}

}  // namespace IOAPIC

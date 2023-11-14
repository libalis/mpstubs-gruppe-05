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

const uint8_t slot_max = 24;

}  // namespace IOAPIC

/*! \file
 *  \brief Handle (disable) the old Programmable Interrupt Controller (PIC)
 */

#pragma once

/*! \brief The Programmable Interrupt Controller (PIC aka 8259A)
 */
namespace PIC {

/*! \brief Initialize the PICs (Programmable Interrupt Controller, 8259A),
 *  such that all 15 hardware interrupts are stored sequentially in the \ref IDT
 *  and the hardware interrupts are disabled (in favor of \ref APIC).
 */
void initialize();

}  // namespace PIC

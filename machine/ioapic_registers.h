/*! \file
 *  \brief Helper structures for interacting with the \ref IOAPIC "I/O APIC".
 */

#pragma once

#include "types.h"

namespace IOAPIC {
	typedef uint32_t Index;
	typedef uint32_t Register;

	extern volatile Index *IOREGSEL_REG;
	extern volatile Register *IOWIN_REG;

	/*! \brief I/O APIC Identification
	 *
	 *  The IOAPICID register is register number 0x0. The I/O APIC's ID will be read from the system configuration
	 *  tables (provided by the BIOS) during boot. The number can be queried by calling \ref APIC::getIOAPICID().
	 *  During initialization, this number must be written to the IOAPICID register.
	 *
	 * \see [IO-APIC manual](intel_ioapic.pdf#page=9), page 9
	 */
	union Identification {
		struct {
			uint32_t     : 24,  ///< Reserved, do not modify
			         id  :  4,  ///< I/O APIC Identification
			             :  4;  ///< Reserved, do not modify
		};
		Register value;
		explicit Identification(Register value) : value(value) {}
	} __attribute__((packed));
	static_assert(sizeof(Identification) == 4, "IOAPIC Identification has wrong size");

	/*! \brief Delivery mode specifies the type of interrupt sent to the CPU. */
	enum DeliveryMode {
		FIXED           = 0,  ///< "ordinary" interrupt; send to ALL cores listed in the destination bit mask
		LOWEST_PRIORITY = 1,  ///< "ordinary" interrupt; send to the lowest priority core from destination mask
		SMI             = 2,  ///< System Management Interrupt; vector number required to be 0
		// Reserved
		NMI             = 4,  ///< Non-Maskable Interrupt, vector number ignored, only edge triggered
		INIT            = 5,  ///< Initialization interrupt (always treated as edge triggered)
		// Reserved
		EXTERN_INT      = 7   ///< external interrupt (only edge triggered)
	};

	/*! \brief Way of interpreting the value written to the destination field. */
	enum DestinationMode {
		PHYSICAL = 0,  ///< Destination contains the physical destination APIC ID
		LOGICAL  = 1   ///< Destination contains a mask of logical APIC IDs
	};

	/*! \brief Interrupt polarity for the redirection-table entry */
	enum Polarity {
		HIGH = 0,  ///< active high
		LOW  = 1   ///< active low
	};

	/*! \brief Trigger mode */
	enum TriggerMode {
		EDGE  = 0,  ///< edge triggered
		LEVEL = 1   ///< level triggered
	};

	/*! \brief Interrupt state */
	enum DeliveryStatus {
		IDLE         = 0,  ///< No activity for this interrupt
		SEND_PENDING = 1   ///< Interrupt will be sent as soon as the bus / LAPIC is ready
	};

	/*! \brief Interrupt masking */
	enum InterruptMask {
		UNMASKED = 0,  ///< Redirection-table entry is active (non-masked)
		MASKED   = 1   ///< Redirection-table entry is inactive (masked)
	};

	/*! \brief Entry in the redirection table.
	 *
	 *  The redirection table begins with I/O APIC register `0x10` and ends at `0x3f`.
	 *
	 *  Each entry has a size of 64 bit, equaling two I/O APIC registers.
	 *  For instance, entry 0 is stored in registers `0x10` and `0x11`, in which the low-order 32 bit
	 *  (equals \ref value_low) and high-order 32 bit (equals \ref value_high) need to be stored.
	 *
	 *  The union defined below provides an overlay allowing convenient modification of individual
	 *  bits, while the 32-bit values \ref value_low and \ref value_high can be used for writing to
	 *  the I/O APIC registers.
	 *
	 * \note [Type punning](https://en.wikipedia.org/wiki/Type_punning#Use_of_union)
	 *       is indeed undefined behavior in C++. However, *gcc* explicitly allows this construct as a
	 *       [language extension](https://gcc.gnu.org/bugs/#nonbugs).
	 *       Some compilers ([other than gcc](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Type%2Dpunning)
	 *       might allow this feature only by disabling strict aliasing (`-fno-strict-aliasing`).
	 *       In \StuBS we use this feature extensively due to the improved code readability.
	 *
	 * \see [IO-APIC manual](intel_ioapic.pdf#page=11), page 11-13
	 */
	union RedirectionTableEntry {
		// @cond ANONYMOUS_STRUCT
		struct {
		// @endcond

			/*! \brief Interrupt vector in the \ref IDT "Interrupt Descriptor Table (IDT)" will be
			 *          activated when the corresponding external interrupt triggers.
			 */
			uint64_t vector : 8;

			/*! \brief The delivery mode denotes the way the interrupts will be delivered to the local CPU
			 *         cores, respectively to their local APICs.
			 *
			 *  For StuBS, we use \ref LOWEST_PRIORITY, as all CPU cores have the same
			 *  priority and we want to distribute interrupts evenly among them.
			 *  It, however, is not guaranteed that this method of load balancing will work on every system.
			 */
			DeliveryMode delivery_mode : 3;

			/*! \brief The destination mode defines how the value stored in \ref destination will be
			 *         interpreted.
			 *
			 * For StuBS, we use \ref LOGICAL
			 */
			DestinationMode destination_mode : 1;

			/*! \brief Delivery status holds the current status of interrupt delivery.
			 *
			 *  \note This field is read only; write accesses to this field will be ignored.
			 */
			DeliveryStatus delivery_status : 1;

			/*! \brief The polarity denotes when an interrupt should be issued.
			 *
			 *  For StuBS, we usually use \ref HIGH (i.e., when the interrupt line is, logically, `1`).
			 */
			Polarity polarity : 1;

			/*! \brief The remote IRR bit indicates whether the local APIC(s) accept the level interrupt.
			 *
			 *  Once the LAPIC sends an \ref LAPIC::endOfInterrupt "End Of Interrupt (EOI)",
			 *  this bit is reset to `0`.
			 *
			 * \note This field is read only and is only meaningful for level-triggered interrupts.
			 */
			uint64_t remote_irr : 1;

			/*! \brief The trigger mode states whether the interrupt signaling is level or edge triggered.
			 *
			 *  StuBS uses \ref EDGE for the Timer, the Keybaord and (optional) serial interface
			 *  need \ref LEVEL
			 */
			TriggerMode trigger_mode : 1;

			/*! \brief Mask or unmask interrupts for a particular, external source.
			 *
			 *  The interrupt mask denotes whether interrupts should be accepted/unmasked
			 *  (value \ref UNMASKED) or ignored/masked (value \ref MASKED).
			 */
			InterruptMask interrupt_mask : 1;

			/*! \brief Reserved, do not modify. */
			uint64_t : 39;

			/*! \brief Interrupt destination.
			 *
			 *  The meaning of destination depends on the destination mode:
			 *  For the logical destination mode, destination holds a bit mask made up of the cores that
			 *  are candidates for receiving the interrupt.
			 *  In the single-core case, this value is `1`, in the multi-core case, the `n` low-order bits
			 *  needs to be set (with `n` being the number of CPU cores, see \ref Core::count() ).
			 *  Setting the `n` low-order bits marks all available cores as candidates for receiving
			 *  interrupts and thereby balancing the number of interrupts between the cores.
			 *
			 *  \note This form of load balancing depends on the hardware's behavior and may not work on all
			 *        systems in the same fashion. Most notably, in QEMU all interrupts are sent to the BSP
			 *        (core 0).
			 */
			uint64_t destination : 8;

		// @cond ANONYMOUS_STRUCT
		} __attribute__((packed));
		// @endcond

		// @cond ANONYMOUS_STRUCT
		struct {
		// @endcond

			Register value_low;   ///< Low-order 32 bits (for the register with the smaller index)
			Register value_high;  ///< High-order 32 bits (for the register with the higher index)
		// @cond ANONYMOUS_STRUCT
		} __attribute__((packed));
		// @endcond

		/*! \brief Constructor for an redirection-table entry
		 *
		 *  Every entry in the redirection table represents an external source of interrupts and has a size
		 *  of 64 bits. Due to the I/O APIC registers being only 32 bits wide, the constructor takes two
		 *  32 bit values.
		 *
		 * \param value_low  First, low-order 32 bit value
		 * \param value_high Second, high-order 32 bit value
		 */
		RedirectionTableEntry(Register value_low, Register value_high) : value_low(value_low), value_high(value_high) {}
	};

	static_assert(sizeof(RedirectionTableEntry) == 8, "IOAPIC::RedirectionTableEntry has wrong size");
}  // namespace IOAPIC

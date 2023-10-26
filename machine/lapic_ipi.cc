#include "machine/lapic_registers.h"

namespace LAPIC {
namespace IPI {

/*! \brief Delivery mode specifies the type of interrupt sent to the CPU. */
enum DeliveryMode {
	FIXED               = 0,  ///< "ordinary" interrupt; send to ALL cores listed in the destination bit mask
	LOWEST_PRIORITY     = 1,  ///< "ordinary" interrupt; send to the lowest priority core from destination mask
	SMI                 = 2,  ///< System Management Interrupt; vector number required to be 0
	// Reserved
	NMI                 = 4,  ///< Non-Maskable Interrupt, vector number ignored, only edge triggered
	INIT                = 5,  ///< Initialization interrupt (always treated as edge triggered)
	INIT_LEVEL_DEASSERT = 5,  ///< Synchronization interrupt
	STARTUP             = 6,  ///< Dedicated Startup-Interrupt (SIPI)
	// Reserved
};

/*! \brief Way of interpreting the value written to the destination field. */
enum DestinationMode {
	PHYSICAL = 0,  ///< Destination contains the physical destination APIC ID
	LOGICAL  = 1   ///< Destination contains a mask of logical APIC IDs
};

/*! \brief Interrupt state */
enum DeliveryStatus {
	IDLE         = 0,  ///< No activity for this interrupt
	SEND_PENDING = 1   ///< Interrupt will be sent as soon as the bus / LAPIC is ready
};

/*! \brief Interrupt level */
enum Level {
	DEASSERT = 0,  ///< Must be zero when DeliveryMode::INIT_LEVEL_DEASSERT
	ASSERT   = 1   ///< Must be one for all other delivery modes
};

/*! \brief Trigger mode for DeliveryMode::INIT_LEVEL_DEASSERT */
enum TriggerMode {
	EDGE_TRIGGERED  = 0,  ///< edge triggered
	LEVEL_TRIGGERED = 1   ///< level triggered
};

/*! \brief Shorthand for commonly used destinations */
enum DestinationShorthand {
	NO_SHORTHAND       = 0,  ///< Use destination field instead of shorthand
	SELF               = 1,  ///< Send IPI to self
	ALL_INCLUDING_SELF = 2,  ///< Send IPI to all including self
	ALL_EXCLUDING_SELF = 3   ///< Send IPI to all except self
};

/*! \brief Interrupt mask */
enum InterruptMask {
	UNMASKED = 0,  ///< Interrupt entry is active (non-masked)
	MASKED   = 1   ///< Interrupt entry is deactivated (masked)
};

/*! \brief Interrupt Command
 *
 *  \see [ISDMv3 10.6.1 Interrupt Command Register (ICR)](intel_manual_vol3.pdf#page=381)
 */
union InterruptCommand {
	struct {
		/*! \brief Interrupt vector in the \ref IDT "Interrupt Descriptor Table (IDT)" will be
		 *          activated when the corresponding external interrupt triggers.
		 *//*! \brief Interrupt vector in the \ref IDT "Interrupt Descriptor Table (IDT)" will be
		 *          activated when the corresponding external interrupt triggers.
		 */
		uint64_t vector : 8;

		/*! \brief The delivery mode denotes the way the interrupts will be delivered to the local CPU
		 *         cores, respectively to their local APICs.
		 *
		 *  For StuBS, we use `DeliveryMode::LowestPriority`, as all CPU cores have the same
		 *  priority and we want to distribute interrupts evenly among them.
		 *  It, however, is not guaranteed that this method of load balancing will work on every system.
		 */
		enum DeliveryMode delivery_mode : 3;

		/*! \brief The destination mode defines how the value stored in `destination` will be
		 *         interpreted.
		 *
		 * For StuBS, we use `DestinationMode::Logical`.
		 */
		enum DestinationMode destination_mode : 1;

		/*! \brief Delivery status holds the current status of interrupt delivery.
		 *
		 *  \note This field is read only; write accesses to this field will be ignored.
		 */
		enum DeliveryStatus delivery_status : 1;

		uint64_t : 1;  ///< reserved

		 /*! \brief The polarity denotes when an interrupt should be issued.
		  *
		  *  For StuBS, we use `Polarity::High` (i.e., when the interrupt line is, logically, 1).
		  */
		enum Level level : 1;

		/*! \brief The trigger mode states whether the interrupt signaling is level or edge triggered.
		 *
		 *  StuBS uses `TriggerMode::Edge` for Keyboard and Timer, the (optional) serial interface,
		 *  however, needs `TriggerMode::Level`.
		 */
		enum TriggerMode trigger_mode : 1;

		uint64_t : 2;  ///< reserved

		enum DestinationShorthand destination_shorthand : 2;

		uint64_t : 36;  ///< Reserved, do not modify

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
	} __attribute__((packed));

	/*! \brief I/O redirection-table entry
	 *
	 *  Every entry in the redirection table represents an external source of interrupts and has a size
	 *  of 64 bits. Due to the I/O APIC registers being only 32 bits wide, the 64-bit value is split in two
	 *  32 bit values.
	 */
	struct {
		Register value_low;   ///< First, low-order register
		Register value_high;  ///< Second, high-order register
	} __attribute__((packed));

	/*! \brief Default constructor */
	InterruptCommand() = default;

	explicit InterruptCommand(uint8_t destination, uint8_t vector = 0,
	                          DestinationMode destination_mode = DestinationMode::PHYSICAL,
	                          DeliveryMode delivery_mode = DeliveryMode::FIXED,
	                          TriggerMode trigger_mode = TriggerMode::EDGE_TRIGGERED,
	                          Level level = Level::ASSERT) {
		readRegister();
		this->vector = vector;
		this->delivery_mode = delivery_mode;
		this->destination_mode = destination_mode;
		this->level = level;
		this->trigger_mode = trigger_mode;
		this->destination_shorthand =  DestinationShorthand::NO_SHORTHAND;
		this->destination = destination;
	}

	InterruptCommand(DestinationShorthand destination_shorthand, uint8_t vector,
	                 DeliveryMode delivery_mode = DeliveryMode::FIXED,
	                 TriggerMode trigger_mode = TriggerMode::EDGE_TRIGGERED,
	                 Level level = Level::ASSERT) {
		readRegister();
		this->vector = vector;
		this->delivery_mode = delivery_mode;
		this->level = level;
		this->trigger_mode = trigger_mode;
		this->destination_shorthand = destination_shorthand;
		this->destination = destination;
	}

	void send() const {
		write(INTERRUPT_COMMAND_REGISTER_HIGH, value_high);
		write(INTERRUPT_COMMAND_REGISTER_LOW, value_low);
	}

	bool isSendPending() {
		value_low = read(INTERRUPT_COMMAND_REGISTER_LOW);
		return delivery_status == DeliveryStatus::SEND_PENDING;
	}

 private:
	void readRegister() {
		while (isSendPending()) {}
		value_high = read(INTERRUPT_COMMAND_REGISTER_HIGH);
	}
};
static_assert(sizeof(InterruptCommand) == 8, "LAPIC Interrupt Command has wrong size");

bool isDelivered() {
	InterruptCommand ic;
	return !ic.isSendPending();
}

void send(uint8_t destination, uint8_t vector) {
	InterruptCommand ic(destination, vector);
	ic.send();
}

void sendGroup(uint8_t logical_destination, uint8_t vector) {
	InterruptCommand ic(logical_destination, vector, DestinationMode::LOGICAL);
	ic.send();
}

void sendAll(uint8_t vector) {
	InterruptCommand ic(DestinationShorthand::ALL_INCLUDING_SELF, vector);
	ic.send();
}

void sendOthers(uint8_t vector) {
	InterruptCommand ic(DestinationShorthand::ALL_EXCLUDING_SELF, vector);
	ic.send();
}

void sendInit(bool assert) {
	LAPIC::IPI::InterruptCommand ic(DestinationShorthand::ALL_EXCLUDING_SELF, 0, DeliveryMode::INIT,
	                                assert ? TriggerMode::EDGE_TRIGGERED : TriggerMode::LEVEL_TRIGGERED,
	                                assert ? Level::ASSERT : Level::DEASSERT);
	ic.send();
}

void sendStartup(uint8_t vector) {
	InterruptCommand ic(DestinationShorthand::ALL_EXCLUDING_SELF, vector, DeliveryMode::STARTUP);
	ic.send();
}

}  // namespace IPI
}  // namespace LAPIC

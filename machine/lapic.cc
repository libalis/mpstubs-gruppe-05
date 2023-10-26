#include "machine/lapic.h"
#include "machine/lapic_registers.h"

namespace LAPIC {

/*! \brief Base Address
 * used with offset to access memory mapped registers
 */
volatile uintptr_t base_address = 0xfee00000;

Register read(Index idx) {
	return *reinterpret_cast<volatile Register *>(base_address + idx);
}

void write(Index idx, Register value) {
	*reinterpret_cast<volatile Register *>(base_address + idx) = value;
}

/*! \brief Local APIC ID (for Pentium 4 and newer)
 *
 * Is assigned automatically during boot and should not be changed.
 *
 * \see [ISDMv3, 10.4.6 Local APIC ID](intel_manual_vol3.pdf#page=371)
 */
union IdentificationRegister {
	struct {
		uint32_t          : 24,  ///< (reserved)
		         apic_id  :  8;  ///< APIC ID
	};
	Register value;

	IdentificationRegister() : value(read(Index::IDENTIFICATION)) {}
} __attribute__((packed));

/*! \brief Local APIC Version
 *
 * \see [ISDMv3 10.4.8 Local APIC Version Register](intel_manual_vol3.pdf#page=373)
 */
union VersionRegister {
	struct {
		uint32_t version                : 8,  ///< 0x14 for P4 and Xeon, 0x15 for more recent hardware
		                                : 8,  ///< (reserved)
		         max_lvt_entry          : 8,  ///< Maximum number of local vector entries
		         suppress_eoi_broadcast : 1,  ///< Support for suppressing EOI broadcasts
		                                : 7;  ///< (reserved)
	};
	Register value;

	VersionRegister() : value(read(Index::VERSION)) {}
} __attribute__((packed));

/*! \brief Logical Destination Register
 *  \see [ISDMv3 10.6.2.2 Logical Destination Mode](intel_manual_vol3.pdf#page=385)
 */
union LogicalDestinationRegister {
	struct {
		uint32_t          : 24,  ///< (reserved)
		         lapic_id :  8;  ///< Logical APIC ID
	};
	Register value;

	LogicalDestinationRegister() : value(read(Index::LOGICAL_DESTINATION)) {}
	~LogicalDestinationRegister() {
		write(Index::LOGICAL_DESTINATION, value);
	}
} __attribute__((packed));

enum Model {
	CLUSTER = 0x0,
	FLAT = 0xf
};

/*! \brief Destination Format Register
 *
 *  \see [ISDMv3 10.6.2.2 Logical Destination Mode](intel_manual_vol3.pdf#page=385)
 */
union DestinationFormatRegister {
	struct {
		uint32_t       : 28;  ///< (reserved)
		Model    model :  4;  ///< Model (Flat vs. Cluster)
	};
	Register value;
	DestinationFormatRegister() : value(read(Index::DESTINATION_FORMAT)) {}
	~DestinationFormatRegister() {
		write(Index::DESTINATION_FORMAT, value);
	}
} __attribute__((packed));

/*! \brief Task Priority Register
 *
 *  \see [ISDMv3 10.8.3.1 Task and Processor Priorities](intel_manual_vol3.pdf#page=391)
 */
union TaskPriorityRegister {
	struct {
		uint32_t task_prio_sub :  4,  ///< Task Priority Sub-Class
		         task_prio     :  4,  ///< Task Priority
		                       : 24;  ///< (reserved)
	};
	Register value;
	TaskPriorityRegister() : value(read(Index::TASK_PRIORITY)) {}
	~TaskPriorityRegister() {
		write(Index::TASK_PRIORITY, value);
	}
} __attribute__((packed));

/*! \brief APIC Software Status for Spurious Interrupt Vector */
enum APICSoftware {
	APIC_DISABLED = 0,
	APIC_ENABLED = 1,
};

/*! \brief Focus Processor Checking for Spurious Interrupt Vector */
enum FocusProcessorChecking {
	CHECKING_ENABLED = 0,
	CHECKING_DISABLED = 1,
};

/*! \brief Suppress End-Of-Interrupt-Broadcast for Spurious Interrupt Vector */
enum SuppressEOIBroadcast {
	BROADCAST = 0,
	SUPPRESS_BROADCAST = 1,
};

/*! \brief Spurious Interrupt Vector Register
 *
 * \see [ISDMv3 10.9 Spurious Interrupt](intel_manual_vol3.pdf#page=394)
 */
union SpuriousInterruptVectorRegister  {
	struct {
		uint32_t spurious_vector   :  8;  ///< Spurious Vector
		APICSoftware apic_software :  1;  ///< APIC Software Enable/Disable
		FocusProcessorChecking focus_processor_checking : 1;  ///< Focus Processor Checking
		uint32_t reserved_1 : 2;
		SuppressEOIBroadcast eoi_broadcast_suppression : 1;
		uint32_t reserved:19;
	};
	Register value;

	SpuriousInterruptVectorRegister() : value(read(Index::SPURIOUS_INTERRUPT_VECTOR)) {}
	~SpuriousInterruptVectorRegister() {
		write(Index::SPURIOUS_INTERRUPT_VECTOR, value);
	}
} __attribute__((packed));
static_assert(sizeof(SpuriousInterruptVectorRegister) == 4, "LAPIC Spurious Interrupt Vector has wrong size");

uint8_t getID() {
	IdentificationRegister ir;
	return ir.apic_id;
}

uint8_t getLogicalID() {
	LogicalDestinationRegister ldr;
	return ldr.lapic_id;
}

uint8_t getVersion() {
	VersionRegister vr;
	return vr.version;
}

void init(uint8_t logical_id) {
	// reset logical destination ID
	// can be set using setLogicalLAPICID()
	LogicalDestinationRegister ldr;
	ldr.lapic_id = logical_id;

	// set task priority to 0 -> accept all interrupts
	TaskPriorityRegister tpr;
	tpr.task_prio = 0;
	tpr.task_prio_sub = 0;

	// set flat delivery mode
	DestinationFormatRegister dfr;
	dfr.model = Model::FLAT;

	// use 255 as spurious vector, enable APIC and disable focus processor
	SpuriousInterruptVectorRegister sivr;
	sivr.spurious_vector = 0xff;
	sivr.apic_software = APICSoftware::APIC_ENABLED;
	sivr.focus_processor_checking = FocusProcessorChecking::CHECKING_DISABLED;
}

void endOfInterrupt() {
	// dummy read
	read(SPURIOUS_INTERRUPT_VECTOR);

	// signal end of interrupt
	write(EOI, 0);
}

}  // namespace LAPIC

/*! \file
 *  \brief Structs and methods related to the \ref ACPI "Advanced Configuration and Power Interface (ACPI)"
 */

#pragma once
#include "types.h"

/*! \brief Abstracts the ACPI standard that provides interfaces for hardware detection, device configuration,
 *         and energy management.
 *  \ingroup io
 *
 *  ACPI is the successor to APM (Advanced Power Management), aiming to give the operating system more control
 *  over the hardware. This extended control, for instance, enables the operating system to assign a particular amount
 *  of energy to every device (e.g., by disabling a device or changing to standby mode).
 *  For this purpose, BIOS and chipset provide a set of tables that describe the system and its components and provide
 *  routines the OS can call.
 *  These tables contain details about the system, such as the number of CPU cores and the LAPIC/IOAPIC, which are
 *  determined during system boot.
 */

namespace ACPI {

/*! \brief Root System Description Pointer (RSDP)
 *
 * The first step to using ACPI is finding the RSDP that is used to find the RSDT / XSDT, which themselves
 * contain pointers to even more tables.
 *
 * On UEFI systems, the RSDP can be found in the EFI_SYSTEM_TABLE; for non-UEFI systems we have to search for the
 * signature 'RSD PTR ' in the EBDA (Extended Bios Data Area) or in the memory area up to `FFFFFh`.
 *
 * \see [ACPI-Specification 5.2.5.3; Root System Description Pointer (RSDP) Structure](acpi.pdf#page=161)
 */

struct RSDP {
	char signature[8];     /* must exactly be equal to 'RSD PTR ' */
	uint8_t checksum;
	char oemid[6];
	uint8_t revision;      /* specifies the ACPI version */
	uint32_t rsdtaddress;  /* physical address of the RSDT */
	uint32_t length;
	uint64_t xsdtaddress;  /* physical address of the XSDT */
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__((packed));

/*! \brief System Description Table Header (SDTH)
 *
 * All System Description Tables (e.g., the RSDT) contain the same entries at the very beginning of
 * the structure, which are abstracted in the SDTH.
 *
 * \see [ACPI-Specification 5.2.6; System Description Table Header](acpi.pdf#page=162)
 */
struct SDTH {
	uint32_t signature; /* table id */
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;

	/* \brief Helper method
	 * \return Pointer to the end of the table
	 */
	void *end() {
		return reinterpret_cast<uint8_t*>(this)+length;
	}
} __attribute__((packed));

/*! \brief Root System Description Table (RSDT)
 *
 * The RSDT can be found in the RSDP. The RSDT contains physical addresses of all other System Description Tables,
 * for example the MADT.
 *
 * \see [ACPI-Specification 5.2.7; Root System Description Table (RSDT)](acpi.pdf#page=167)
 */

struct RSDT : SDTH {
	uint32_t entries[];
} __attribute__((packed));

/*! \brief Extended System Description Table (XSDT)
 *
 * Like RSDT, but contains 64-bit instead of 32-bit addresses.
 *
 * \see [ACPI-Specification 5.2.8; Extended System Description Table (XSDT)](acpi.pdf#page=168)
 */

struct XSDT : SDTH {
	uint64_t entries[];
} __attribute__((packed));

/*! \brief Helper structure
 *
 * Is used for accessing the substructures present in SRAT / MADT.
 *
 */
struct SubHeader {
	uint8_t type;
	uint8_t length;

	/* Method to traverse multiple substructures  */
	SubHeader *next() {
		return reinterpret_cast<SubHeader*>(reinterpret_cast<uint8_t*>(this)+length);
	}
} __attribute__((packed));

/*! \brief Multiple APIC Description Table (MADT)
 *
 * Describes all interrupt controllers present within the system. Is used to obtain the IDs of the APICs, along with
 * the number of available processor cores.
 *
 * \see [ACPI-Specification 5.2.12; Multiple APIC Description Table (MADT)](acpi.pdf#page=193)
 */
struct MADT : SDTH {
	uint32_t local_apic_address;
	uint32_t flags_pcat_compat:1,
		flags_reserved:31;

	/* method to access the first subheader */
	SubHeader *first() {
		return reinterpret_cast<SubHeader*>(reinterpret_cast<uint8_t*>(this)+sizeof(MADT));
	}
} __attribute__((packed));

enum class AddressSpace : uint8_t {
	MEMORY = 0x0,
	IO     = 0x1,
};

/*! \brief ACPI address format
 *
 * The ACPI standard defines its own address format that is able to handle addresses both in memory address space,
 * as well as IO-port address space.
 */
struct Address {
	AddressSpace address_space;
	uint8_t register_bit_width;
	uint8_t register_bit_offset;
	uint8_t reserved;
	uint64_t address;
} __attribute__((packed));

// Multiple APIC Definition Structure
namespace MADS {
enum Type {
	Type_LAPIC = 0,
	Type_IOAPIC = 1,
	Type_Interrupt_Source_Override = 2,
	Type_LAPIC_Address_Override = 5,
};

/*! \brief Processor Local APIC (LAPIC) Structure
 *
 * Represents a physical processor along with its local interrupt controller.
 * The MADT contains a LAPIC structure for every processor available in the system.
 *
 * \see [ACPI-Specification 5.2.12.2; Processor Local APIC Structure](acpi.pdf#page=195)
 */
struct LAPIC : SubHeader {
	uint8_t acpi_processor_id;
	uint8_t apic_id;
	uint32_t flags_enabled  :  1,
	         flags_reserved : 31; /* must be 0 */
} __attribute__((packed));

/*! \brief I/O APIC Structure
 *
 * Represents an I/O-APIC.
 * The MADT contains an IOAPIC structure for every I/O APIC present in the system.
 *
 * \see [ACPI-Specification 5.2.12.3; I/O APIC Structure](acpi.pdf#page=196)
 */

struct IOAPIC : SubHeader {
	uint8_t ioapic_id;
	uint8_t reserved;
	uint32_t ioapic_address;
	uint32_t global_system_interrupt_base;
} __attribute__((packed));

/*! \brief Interrupt Source Override Structure
 *
 * Is required to describe differences between the IA-PC standard interrupt definition and the actual
 * hardware implementation.
 *
 * \see [ACPI-Specification 5.2.12.5; Interrupt Source Override Structure](acpi.pdf#page=197)
 */
struct Interrupt_Source_Override : SubHeader {
	uint8_t bus;
	uint8_t source;
	uint32_t global_system_interrupt;
	uint16_t flags_polarity     :  2,
	         flags_trigger_mode :  2,
	         flags_reserved     : 12; /* must be 0 */
} __attribute__((packed));

/*! \brief Local APIC Address Override Structure
 *
 * Support for 64-bit systems is achieved by replacing the 32-bit physical LAPIC address stored in the MADT
 * with the corresponding 64-bit address.
 *
 * \see [ACPI-Specification 5.2.12.8; Local APIC Address Override Structure](acpi.pdf#page=199)
 */

struct LAPIC_Address_Override : SubHeader {
	uint16_t reserved;
	union {
		uint64_t lapic_address;
		struct {
			uint32_t lapic_address_low;
			uint32_t lapic_address_high;
		} __attribute__((packed));
	};
} __attribute__((packed));

}  // namespace MADS

/*! \brief Initialize the ACPI description table
 *
 * Searches physical memory ranges o 16-byte boundaries for a valid Root System Description Pointer (RSDP)
 * structure signature and checksum.
 * If present, the superseding Extended System Description Table (XSDT) is used.
 *
 * \see [ACPI-Specification 5.2.5 Root System Description Pointer (RSDP)](acpi.pdf#page=160)
 * \see [ACPI-Specification 5.2.8 Extended System Description Table (XSDT)](acpi.pdf#page=168)
 */
bool init();

/*! \brief Number of entries in the description table
 */
unsigned count();

/*! \brief Get entry of description table by index
 *
 * \param num index in description table
 * \return Pointer to corresponding entry or `nullptr` if not available
 */
SDTH *get(unsigned num);

/*! \brief Get entry of description table by four character identifier
 *
 * \param a first character of identifier
 * \param b second character of identifier
 * \param c third character of identifier
 * \param d forth and last character of identifier
 * \return Pointer to corresponding entry or `nullptr` if not available
 */
SDTH *get(char a, char b, char c, char d);

/*! \brief Retrieve the revision from the Root System Description Pointer (RSDP)
 */
int revision();

}  // namespace ACPI

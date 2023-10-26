#include "machine/acpi.h"
#include "debug/output.h"

namespace ACPI {

static RSDP* rsdp = 0;
static RSDT* rsdt = 0;
static XSDT* xsdt = 0;

const char * RSDP_SIGNATURE = "RSD PTR ";

static int checksum(const void *pos, unsigned len) {
	const uint8_t *mem = reinterpret_cast<const uint8_t*>(pos);
	uint8_t sum = 0;
	for (unsigned i = 0; i < len; i++) {
		sum += mem[i];
	}

	return sum;
}

static const RSDP* findRSDP(const void *pos, unsigned len) {
	/* since the RSDP is 16-Byte aligned, we only need to check
	   every second 64bit memory block */
	for (unsigned block = 0; block < len / 8; block += 2) {
		const uint64_t *mem = reinterpret_cast<const uint64_t*>(pos) + block;
		if (*mem == *reinterpret_cast<const uint64_t *>(RSDP_SIGNATURE)) {
			const RSDP *rsdp = reinterpret_cast<const RSDP*>(mem);
			/* ACPI Specification Revision 4.0a: 5.2.5.3*/
			if ((rsdp->revision == 0 && checksum(mem, 20) == 0) ||
			    (rsdp->length > 20 && checksum(mem, rsdp->length) == 0)) {
				return rsdp;
			}
		}
	}
	return 0;
}

bool init() {
	/* ACPI Specification Revision 4.0a:
	 * 5.2.5.1 Finding the RSDP on IA-PC Systems
	 * OSPM finds the Root System Description Pointer (RSDP) structure by
	 * searching physical memory ranges on 16-byte boundaries for a valid
	 * Root System Description Pointer structure signature and checksum
	 * match as follows:
	 * * The first 1 KB of the Extended BIOS Data Area (EBDA). For EISA or
	 *   MCA systems, the EBDA can be found in the two-byte location 40:0Eh
	 *   on the BIOS data area.
	 * * The BIOS read-only memory space between 0E0000h and 0FFFFFh.
	 */
	volatile uintptr_t ebda_base_address = 0x40e;
	const uintptr_t ebda = static_cast<uintptr_t>(*reinterpret_cast<uint32_t *>(ebda_base_address));
	const RSDP *rsdp = findRSDP(reinterpret_cast<void*>(ebda), 1024);
	if (rsdp == nullptr) {
		rsdp = findRSDP(reinterpret_cast<void*>(0xe0000), 0xfffff-0xe0000);
	}

	if (rsdp == nullptr) {
		DBG_VERBOSE << "No ACPI!" << endl;
		return false;
	}
	rsdt = reinterpret_cast<RSDT*>(static_cast<uintptr_t>(rsdp->rsdtaddress));

	/* If the XSDT is present we must use it; see:
	 * ACPI Specification Revision 4.0a:
	 * "An ACPI-compatible OS must use the XSDT if present."
	 */
	if (rsdp->revision != 0 && rsdp->length >= 36) {
		xsdt = reinterpret_cast<XSDT*>(rsdp->xsdtaddress);
	}
	DBG_VERBOSE << "ACPI revision " <<  rsdp->revision << endl;
	for (unsigned i = 0; i != count(); ++i) {
		SDTH *sdt = get(i);
		if (sdt != nullptr) {
			char *c = reinterpret_cast<char*>(&sdt->signature);
			DBG_VERBOSE << i << ". " << c[0] << c[1] << c[2] << c[3] << " @ " << reinterpret_cast<void*>(sdt) << endl;
		}
	}
	return true;
}

unsigned count() {
	if (xsdt != nullptr) {
		return (xsdt->length-36)/8;
	} else if (rsdt != nullptr) {
		return (rsdt->length-36)/4;
	} else {
		return 0;
	}
}

SDTH *get(unsigned num) {
	if (xsdt != nullptr) {
		SDTH *entry = reinterpret_cast<SDTH*>(xsdt->entries[num]);
		if (checksum(entry, entry->length) == 0) {
			return entry;
		}
	} else if (rsdt != nullptr) {
		SDTH *entry = reinterpret_cast<SDTH*>(static_cast<uintptr_t>(rsdt->entries[num]));
		if (checksum(entry, entry->length) == 0) {
			return entry;
		}
	}
	return 0;
}

SDTH *get(char a, char b, char c, char d) {
	union {
		char signature[4];
		uint32_t value;
	};
	signature[0] = a;
	signature[1] = b;
	signature[2] = c;
	signature[3] = d;

	if (xsdt != nullptr) {
		for (unsigned i = 0; i < count(); i++) {
			SDTH *entry = reinterpret_cast<SDTH*>(xsdt->entries[i]);
			if (entry->signature == value && checksum(entry, entry->length) == 0) {
				return entry;
			}
		}
	} else if (rsdt != nullptr) {
		for (unsigned i = 0; i < count(); i++) {
			SDTH *entry = reinterpret_cast<SDTH*>(static_cast<uintptr_t>(rsdt->entries[i]));
			if (entry->signature == value && checksum(entry, entry->length) == 0) {
				return entry;
			}
		}
	}
	return 0;
}

int revision() {
	return rsdp != nullptr ? rsdp->revision : -1;
}

}  // namespace ACPI

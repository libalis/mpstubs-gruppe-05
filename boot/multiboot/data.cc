#include "boot/multiboot/data.h"

/*! \brief Multiboot Information Structure according to Specification
 * \see [Multiboot Specification]{#multiboot}
 */
struct multiboot_info {
	/*! \brief Helper Structure
	 */
	struct Array {
		uint32_t size;  ///< Length
		uint32_t addr;  ///< Begin (physical address)
	} __attribute__((packed));

	enum Flag : uint32_t {
		Memory          = 1U << 0,   ///< is there basic lower/upper memory information?
		BootDev         = 1U << 1,   ///< is there a boot device set?
		CmdLine         = 1U << 2,   ///< is the command-line defined?
		Modules         = 1U << 3,   ///< are there modules to do something with?
		/* These next two are mutually exclusive */
		SymbolTable     = 1U << 4,   ///< is there an a.out symbol table loaded?
		SectionHeader   = 1U << 5,   ///< is there an ELF section header table?

		MemoryMap       = 1U << 6,   ///< is there a full memory map?
		DriveInfo       = 1U << 7,   ///< Is there drive info?
		ConfigTable     = 1U << 8,   ///< Is there a config table?
		BootLoaderName  = 1U << 9,   ///< Is there a boot loader name?
		ApmTable        = 1U << 10,  ///< Is there a APM table?

		// Is there video information?
		VbeInfo         = 1U << 11,  ///< Vesa bios extension
		FramebufferInfo = 1U << 12   ///< Framebuffer
	} flags;

	/*! \brief Available memory retrieved from BIOS
	 */
	struct {
		uint32_t lower;    ///< Amount of memory below 1 MiB in kilobytes
		uint32_t upper;    ///< Amount of memory above 1 MiB in kilobytes
	} mem __attribute__((packed));
	uint32_t boot_device;  ///< "root" partition
	uint32_t cmdline;      ///< Kernel command line
	Array mods;            ///< List of boot modules
	union {
		/*! \brief Symbol table for kernel in a.out format
		 */
		struct {
			uint32_t tabsize;
			uint32_t strsize;
			uint32_t addr;
			uint32_t reserved;
		} aout_symbol_table  __attribute__((packed));

		/*! \brief Section header table for kernel in ELF
		 */
		struct {
			uint32_t num;    ///< Number of entries
			uint32_t size;   ///< Size per entry
			uint32_t addr;   ///< Start of the header table
			uint32_t shndx;  ///< String table index
		} elf_section_header_table  __attribute__((packed));
	};

	struct Array mmap;          ///< Memory Map
	struct Array drives;        ///< Drive Information
	uint32_t config_table;      ///< ROM configuration table
	uint32_t boot_loader_name;  ///< Boot Loader Name
	uint32_t apm_table;         ///< APM table

	struct Multiboot::VBE vbe;   ///< VBE Information
	struct Multiboot::Framebuffer framebuffer;  ///< Framebuffer information

	/*! \brief Check if setting is available
	 * \param flag Flag to check
	 * \return `true` if available
	 */
	bool has(enum Flag flag) const {
		return (flags & flag) != 0;
	}
} __attribute__((packed));
assert_size(multiboot_info, 116);

/*! \brief The pointer to the multiboot structures will be assigned in the assembler startup code (boot/startup.asm)
 */
struct multiboot_info *multiboot_addr = 0;

namespace Multiboot {
Module * getModule(unsigned i) {
	if (multiboot_addr != nullptr &&
	    multiboot_addr->has(multiboot_info::Flag::Modules) &&
		i < multiboot_addr->mods.size) {
		return i + reinterpret_cast<Module*>(static_cast<uintptr_t>(multiboot_addr->mods.addr));
	} else {
		return nullptr;
	}
}

unsigned getModuleCount() {
	return multiboot_addr->mods.size;
}

void * Memory::getStartAddress() const {
	if (sizeof(void*) == 4 && (addr >> 32) != 0) {
		return reinterpret_cast<void*>(addr & 0xffffffff);
	} else {
		return reinterpret_cast<void*>(static_cast<uintptr_t>(addr));
	}
}

void * Memory::getEndAddress() const {
	uint64_t end = addr + len;
	if (sizeof(void*) == 4 && (end >> 32) != 0) {
		return reinterpret_cast<void*>(end & 0xffffffff);
	} else {
		return reinterpret_cast<void*>(static_cast<uintptr_t>(end));
	}
}

bool Memory::isAvailable() const {
	return type == AVAILABLE;
}

Memory * Memory::getNext() const {
	if (multiboot_addr != nullptr && multiboot_addr->has(multiboot_info::Flag::MemoryMap)) {
		uintptr_t next = reinterpret_cast<uintptr_t>(this) + size + sizeof(size);
		if (next < multiboot_addr->mmap.addr + multiboot_addr->mmap.size) {
			return reinterpret_cast<Memory *>(next);
		}
	}
	return nullptr;
}

Memory * getMemoryMap() {
	if (multiboot_addr != nullptr &&
	    multiboot_addr->has(multiboot_info::Flag::MemoryMap) &&
	    multiboot_addr->mmap.size > 0) {
		return reinterpret_cast<Memory *>(static_cast<uintptr_t>(multiboot_addr->mmap.addr));
	} else {
		return nullptr;
	}
}

char * getCommandLine() {
	return reinterpret_cast<char*>(static_cast<uintptr_t>(multiboot_addr->cmdline));
}

char * getBootLoader() {
	return reinterpret_cast<char*>(static_cast<uintptr_t>(multiboot_addr->boot_loader_name));
}

VBE * getVesaBiosExtensionInfo() {
	if (multiboot_addr != nullptr && multiboot_addr->has(multiboot_info::Flag::VbeInfo)) {
		return &(multiboot_addr->vbe);
	} else {
		return nullptr;
	}
}

Framebuffer * getFramebufferInfo() {
	if (multiboot_addr != nullptr && multiboot_addr->has(multiboot_info::Flag::FramebufferInfo)) {
		return &(multiboot_addr->framebuffer);
	} else {
		return nullptr;
	}
}
}  // namespace Multiboot

/*! \file
 *  \brief \ref Multiboot Interface
 */
#pragma once

#include "types.h"
#include "compiler/fix.h"
#include "debug/assert.h"

/*! \brief Interface for Multiboot
 *
 * Due to historical reasons, a normal BIOS allows you to do quite an egg dance
 * until you finally reach the actual kernel (especially with only 512 bytes
 * available in the master boot record...).
 * Fortunately, there are [boot loaders](https://wiki.osdev.org/Bootloader) that
 * (partly) do this ungrateful job for you:
 * They load your kernel into memory, switch (the bootstrap processor) to
 * protected mode (32 bit) and jump to the entry point of our kernel -- saving
 * you a lot of boring (or enlightening?) work: reading ancient systems documentation.
 * One of the most famous representatives is the
 * [Grand Unified Bootloader (GRUB)](https://www.gnu.org/software/grub/), which
 * is also the reference implementation of the \ref multiboot "Multiboot Specification".
 *
 * A Multiboot compliant boot loader will prepare the system according to your
 * needs and can hand you a lot of useful information (e.g. references to
 * initial ramdisks).
 *
 * However, you have to inform the loader that you are also compliant to the
 * specification, and (if required) instruct the loader to adjust specific
 * settings (e.g. the graphics mode).
 *
 * For this purpose you have to configure the beginning of the kernel (the first
 * 8192 bytes of the kernel binary) accordingly (see `compiler/section.ld`) --
 * this is were the boot loader will search for a magic header and parse the
 * subsequent entries containing the desired system configuration.
 * In StuBS these flags are set in `boot/multiboot/config.inc` and the header
 * structure is generated in `boot/multiboot/header.asm`.
 *
 * The first step in your \ref startup_bsp() "kernel entry function" is saving
 * the pointer to the struct with the information from the boot loader
 * (transferred via register `ebx`) -- and \ref Multiboot provides you the
 * interface to comfortably access its contents!
 */
namespace Multiboot {
/*! \brief Boot Module
 * (also known as `initrd` = initial Ramdisk)
 *
 * \see \ref multiboot-boot-modules "1.7 Boot modules"
 * \see \ref multiboot-boot-info "3.3 Boot information format"
 */
class Module {
	uint32_t start;                    ///< Start address
	uint32_t end;                      ///< End address (excluded)
	uint32_t cmdline;                  ///< commandline parameter
	uint32_t pad UNUSED_STRUCT_FIELD;  ///< alignment; must be 0

 public:
	/*! \brief Get start of this boot module
	 * \return Pointer to begin of modules physical address
	 */
	void * getStartAddress() const {
		return reinterpret_cast<void*>(static_cast<uintptr_t>(start));
	}

	/*! \brief Get end of this boot module
	 * \return Pointer beyond the modules physical address
	 */
	void * getEndAddress() const {
		return  reinterpret_cast<void*>(static_cast<uintptr_t>(end));
	}

	/*! \brief Get the size of this boot module
	 * \return Module size in bytes (difference of end and start address)
	 */
	size_t getSize() const {
		return static_cast<size_t>(end-start);
	}

	/*! \brief Get the command line for this module
	 * \return pointer to zero terminated string
	 */
	char * getCommandLine() const {
		return  reinterpret_cast<char*>(static_cast<uintptr_t>(cmdline));
	}
} __attribute__((packed));
assert_size(Module, 16);

/*! \brief Retrieve a certain boot module
 * \param i boot module number
 * \return Pointer to structure with boot module information
 */
Module * getModule(unsigned i);

/*! \brief Get the number of modules
 * \return Pointer to structure with boot module information
 */
unsigned getModuleCount();

/*! \brief Get the kernel command line
 * \return pointer to zero terminated string
 */
char * getCommandLine();

/*! \brief Get the name of the boot loader
 * \return pointer to zero terminated string
 */
char * getBootLoader();

/*! \brief Memory Map
 *
 * The boot loader queries the BIOS for a memory map and stores its result in
 * (something like) a linked list. However, this list may not be complete,
 * can have contradictory entries and does not take the location of your kernel
 * or any boot modules into account.
 * (Anyways, it is still the best memory map you will have in StuBS...)
 *
 * \note Needs to be enabled explicitly by setting the `MULTIBOOT_MEMORY_INFO` flag
 *       in the multiboot header (see `boot/multiboot/config.inc`)!
 *
 * \see [Detecting Memory](https://wiki.osdev.org/Detecting_Memory_(x86))
 */
class Memory {
	uint32_t size;  ///< Size of this entry (can exceed size of the class, rest will be padding bits)
	uint64_t addr;  ///< Begin of memory area
	uint64_t len;   ///< length of the memory area

	/*! \brief Usage Type
	 */
	enum Type : uint32_t {
		AVAILABLE = 1,  ///< Memory is available and usable in kernel
		RESERVED  = 2,  ///< Memory is reserved (without further explanation)
		ACPI      = 3,  ///< Memory may be reclaimed by ACPI
		NVS       = 4,  ///< Memory is non volatile storage for ACPI
		BADRAM    = 5   ///< Area contains bad memory
	} type;

 public:
	/*! \brief Get start of this memory area
	 * \return Pointer to begin of the physical address of the memory area
	 */
	void * getStartAddress() const;

	/*! \brief Get end of this memory area
	 * \return Pointer beyond the physical address of this memory area
	 */
	void * getEndAddress() const;

	/*! \brief Is the memory marked as usable
	 * \return `true` if available, `false` if not usable.
	 */
	bool isAvailable() const;

	/*! \brief Get the next memory area
	 * \return pointer to the next memory area entry or `nullptr` if last area
	 */
	Memory * getNext() const;
} __attribute__((packed));
assert_size(Memory, 24);

/*! \brief Retrieve the first entry of the memory map
 */
Memory * getMemoryMap();

/*! \brief Video mode: Vesa BIOS Extension
 *
 * \see [VESA BIOS Extension (VBE) Core Functions (Version 3)](vbe3.pdf)
 */
struct VBE {
	uint32_t control_info;   ///< Pointer to VBE control information
	uint32_t mode_info;      ///< Pointer to VBE mode information
	uint16_t mode;           ///< Selected video mode (as defined in the standard)
	uint16_t interface_seg;  ///< Protected mode interface (unused)
	uint16_t interface_off;  ///< Protected mode interface (unused)
	uint16_t interface_len;  ///< Protected mode interface (unused)
} __attribute__((packed));
assert_size(VBE, 16);

/*! \brief Get pointer to Vesa BIOS Extension information
 *
 * \note Only available if the `MULTIBOOT_VIDEO_MODE` flag was explicitly set
 *       in the multiboot header (see `boot/multiboot/config.inc`)!
 */
VBE * getVesaBiosExtensionInfo();

/*! \brief Video mode: Framebuffer
 *
 * This beautiful structure contains everything required for using the graphic
 * framebuffer in a very handy manner -- however, it may not be well supported
 * by current boot loaders...
 * These information can be retrieved from \ref VBE as well, though you then
 * have to parse these huge structures containing a lot of useless stuff.
 */
struct Framebuffer {
	uint64_t address;  ///< Physical address of the framebuffer
	uint32_t pitch;    ///< Number of bytes per row
	uint32_t width;    ///< Width of framebuffer
	uint32_t height;   ///< Height of framebuffer
	uint8_t bpp;       ///< Bits per pixel
	enum Type : uint8_t {
		INDEXED = 0,  ///< Using a custom color palette
		RGB = 1,      ///< Standard red-green-blue
		EGA_TEXT = 2  ///< Enhanced Graphics Adapter color palette
	} type;
	union {
		/*! \brief For INDEXED type
		 */
		struct {
			uint32_t palette_addr;        ///< Address of an array with RGB values
			uint16_t palette_num_colors;  ///< Number of colors (in array above)
		} __attribute__((packed));

		/*! \brief For RGB type
		 */
		struct {
			uint8_t offset_red;    ///< Offset of red value
			uint8_t bits_red;      ///< Bits used in red value
			uint8_t offset_green;  ///< Offset of green value
			uint8_t bits_green;    ///< Bits used in green value
			uint8_t offset_blue;   ///< Offset of blue value
			uint8_t bits_blue;     ///< Bits used in blue value
		} __attribute__((packed));
	} __attribute__((packed));
} __attribute__((packed));
assert_size(Framebuffer, 28);

/*! \brief Get pointer to framebuffer information
 *
 * \note Only available if the `MULTIBOOT_VIDEO_MODE` flag was explicitly set
 *       in the multiboot header (see `boot/multiboot/config.inc`)!
 */
Framebuffer * getFramebufferInfo();
}  // namespace Multiboot

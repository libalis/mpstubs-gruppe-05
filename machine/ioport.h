/*! \file
 *  \brief \ref IOPort provides access to the x86 IO address space
 */

#pragma once

#include "types.h"

/*! \brief Abstracts access to the I/O address space
 *
 *  x86 PCs have a separated I/O address space that is accessible only via the machine instructions `in` and `out`.
 *  An IOPort object encapsulates the corresponding address in the I/O address space and can be used for byte or
 *  word-wise reading or writing.
 */

class IOPort {
	/*! \brief Address in I/O address space
	 *
	 */
	uint16_t address;

 public:
	/*! \brief Constructor
	 *  \param addr Address from the I/O address space
	 */
	explicit IOPort(uint16_t addr) : address(addr) {}

	/*! \brief Write one byte to the I/O port
	 *  \param val The value to be written
	 */
	void outb(uint8_t val) const {
		asm volatile(
			"out %%al, %%dx\n\t"
			:
			: "a"(val), "d"(address)
			:
		);
	}

	/*! \brief Write one word (2 bytes) to the I/O port
	 *  \param val The value to be written
	 */
	void outw(uint16_t val) const {
		asm volatile(
			"out %%ax, %%dx\n\t"
			:
			:"a"(val), "d"(address)
			:
		);
	}

	/*! \brief Read one byte from the I/O port
	 *  \return Read byte
	 */
	uint8_t inb() const {
		uint8_t out = 0;

		asm volatile(
			"in %%dx, %%al\n\t"
			:"=a"(out)
			:"d"(address)
			:
		);

		return out;
	}

	/*! \brief Read one word (2 bytes) from the I/O port
	 *  \return Read word (2 bytes)
	 */
	uint16_t inw() const {
		uint16_t out = 0;

		asm volatile(
			"inw %%dx, %%ax\n\t"
			:"=a"(out)
			:"d"(address)
			:
		);

		return out;
	}
};

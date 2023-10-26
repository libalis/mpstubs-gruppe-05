#include "machine/cmos.h"

#include "machine/core.h"
#include "machine/ioport.h"

namespace CMOS {
static IOPort address(0x70);
static IOPort data(0x71);

namespace NMI {
static const uint8_t mask = 0x80;
// Cache NMI to speed things up
static bool disabled = false;

void enable() {
	bool status = Core::Interrupt::disable();
	uint8_t value = address.inb();
	value &= ~mask;
	address.outb(value);
	Core::Interrupt::restore(status);
	disabled = false;
}

void disable() {
	bool status = Core::Interrupt::disable();
	uint8_t value = address.inb();
	value |= mask;
	address.outb(value);
	Core::Interrupt::restore(status);
	disabled = true;
}

bool isEnabled() {
	disabled = (address.inb() & mask) != 0;
	return !disabled;
}
}  // namespace NMI

static void setAddress(enum Register reg) {
	uint8_t value = reg;
	// The highest bit controls the Non Maskable Interrupt
	// so we don't want to accidentally change it.
	if (NMI::disabled) {
		value |= NMI::mask;
	} else {
		value &= ~NMI::mask;
	}
	address.outb(value);
}

uint8_t read(enum Register reg) {
	setAddress(reg);
	return data.inb();
}

void write(enum Register reg, uint8_t value) {
	setAddress(reg);
	data.outb(value);
}

}  // namespace CMOS

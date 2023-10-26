#include "machine/pic.h"
#include "machine/ioport.h"
#include "types.h"

namespace PIC {

void initialize() {
	// Access primary & secondary PIC via two ports each
	IOPort primary_port_a(0x20);
	IOPort primary_port_b(0x21);
	IOPort secondary_port_a(0xa0);
	IOPort secondary_port_b(0xa1);

	// Initialization Command Word 1 (ICW1)
	// Basic PIC configuration, starting initialization
	enum InitializationCommandWord1 {
		ICW4_NEEDED           = 1 << 0,   // use Initialization Command Word 4
		SINGLE_MODE           = 1 << 1,   // Single or multiple (cascade mode)  8259A
		ADDRESS_INTERVAL_HALF = 1 << 2,   // 4 or 8 bit interval between the interrupt vector locations
		LEVEL_TRIGGERED       = 1 << 3,   // Level or edge triggered
		ALWAYS_1              = 1 << 4,
	};
	const uint8_t icw1 = InitializationCommandWord1::ICW4_NEEDED
	                   | InitializationCommandWord1::ALWAYS_1;
	// ICW1 in port A (each)
	primary_port_a.outb(icw1);
	secondary_port_a.outb(icw1);

	// Initialization Command Word 2 (ICW2):
	// Configure interrupt vector base offset in port B
	primary_port_b.outb(0x20);     // Primary: IRQ Offset 32
	secondary_port_b.outb(0x28);   // Secondary: IRQ Offset 40

	// Initialization Command Word 3 (ICW3):
	// Configure pin on primary PIC connected to secondary PIC
	const uint8_t pin = 2;           // Secondary connected on primary pin 2
	primary_port_b.outb(1 << pin);   // Pin as bit mask for primary
	secondary_port_b.outb(pin);      // Pin as value (ID) for secondary

	// Initialization Command Word 4 (ICW4)
	// Basic PIC configuration, starting initialization
	enum InitializationCommandWord4 {
		MODE_8086            = 1 << 0,   // 8086/8088 or 8085 mode
		AUTO_EOI             = 1 << 1,   // Single or multiple (cascade mode)  8259A
		BUFFER_PRIMARY       = 1 << 2,   // Primary or secondary buffering
		BUFFERED_MODE        = 1 << 3,   // Enable or disable buffering (for primary or secondary above)
		SPECIAL_FULLY_NESTED = 1 << 4    // Special or non special fully nested
	};
	const uint8_t icw4 = InitializationCommandWord4::MODE_8086
	                   | InitializationCommandWord4::AUTO_EOI;
	// ICW3 in port B (each)
	primary_port_b.outb(icw4);
	secondary_port_b.outb(icw4);

	// Operation Control Word 1 (OCW1):
	// Disable (mask) all hardware interrupts on both legacy PICs (we'll use APIC)
	secondary_port_b.outb(0xff);
	primary_port_b.outb(0xff);
}

}  // namespace PIC

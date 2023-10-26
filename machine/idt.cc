#include "machine/idt.h"
#include "machine/gdt.h"
#include "machine/core_interrupt.h"

namespace IDT {

// Interrupt Descriptor stored in the Interrupt-Descriptor Table (IDT)
struct alignas(8) InterruptDescriptor {
	uint16_t address_low;         ///< lower interrupt function offset
	uint16_t selector;            ///< code segment selector in GDT or LDT
	union {
		struct {
			uint8_t ist     : 3;  ///< IST Index (64 bit)
			uint8_t         : 5;  ///< unused, has to be 0
			Gate type       : 3;  ///< gate type
			GateSize size   : 1;  ///< gate size
			uint8_t         : 1;  ///< unused, has to be 0
			DPL dpl         : 2;  ///< descriptor privilege level
			uint8_t present : 1;  ///< present: 1 for interrupts
		} __attribute__((packed));
		uint16_t flags;
	};
	uint64_t address_high : 48;   ///< higher interrupt function offset
	uint64_t              : 0;    ///< fill until aligned with 64 bit
} __attribute__((packed));

// Interrupt Descriptor Table, 8 Byte aligned
static struct InterruptDescriptor idt[256];

// Struct used for loading (the address of) the Interrupt Descriptor Table into the IDT-Register
struct Register {
	uint16_t limit;  // Address of the last valid byte (relative to base)
	struct InterruptDescriptor * base;
	explicit Register(uint8_t max = 255) {
		limit = (max + static_cast<uint16_t>(1)) * sizeof(InterruptDescriptor) - 1;
		base  = idt;
	}
} __attribute__((packed));

static_assert(sizeof(InterruptDescriptor) == 16, "IDT::InterruptDescriptor has wrong size");
static_assert(sizeof(Register) == 10, "IDT::Register has wrong size");
static_assert(alignof(decltype(idt)) % 8 == 0, "IDT must be 8 byte aligned!");

void load() {
	// Create structure required for writing to idtr and load via lidt
	Register idtr(Core::Interrupt::VECTORS - 1);
	asm volatile("lidt %0\n\t" :: "m"(idtr) );
}

void handle(uint8_t vector, void * handler, enum Gate type, enum GateSize size, enum DPL dpl, bool present) {
	struct InterruptDescriptor &item = idt[vector];
	item.selector = GDT::SEGMENT_KERNEL_CODE * sizeof(GDT::SegmentDescriptor);
	item.type     = type;
	item.size     = size;
	item.dpl      = dpl;
	item.present  = present ? 1 : 0;

	uintptr_t address = reinterpret_cast<uintptr_t>(handler);
	item.address_low  = address & 0xffff;
	item.address_high = (address >> 16) & 0xffffffffffff;
}

}  // namespace IDT

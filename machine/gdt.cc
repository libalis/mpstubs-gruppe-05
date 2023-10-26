#include "machine/gdt.h"
#include "machine/core.h"
#include "debug/assert.h"
#include "debug/output.h"

namespace GDT {

// The static 32-bit Global Descriptor Table (GDT)
alignas(16) static SegmentDescriptor protected_mode[] = {
	// NULL descriptor
	{},

	// Global code segment von 0-4GB
	{	/* base  = */ 0x0,
		/* limit = */ 0xFFFFFFFF,
		/* code  = */ true,
		/* ring  = */ 0,
		/* size  = */ SIZE_32BIT },

	// Global data segment von 0-4GB
	{	/* base  = */ 0x0,
		/* limit = */ 0xFFFFFFFF,
		/* code  = */ false,
		/* ring  = */ 0,
		/* size  = */ SIZE_32BIT },

};
extern "C" constexpr Pointer gdt_protected_mode_pointer(protected_mode);

// The static 64-bit Global Descriptor Table (GDT)
// \see [ISDMv3 3.2.4 Segmentation in IA-32e Mode](intel_manual_vol3.pdf#page=91)
alignas(16) static SegmentDescriptor long_mode[] = {
	// Null segment
	{},

	// Global code segment
	{	/* base  = */ 0x0,
		/* limit = */ 0x0,  // ignored
		/* code  = */ true,
		/* ring  = */ 0,
		/* size  = */ SIZE_64BIT_CODE },

	// Global data segment
	{	/* base  = */ 0x0,
		/* limit = */ 0x0,  // ignored
		/* code  = */ false,
		/* ring  = */ 0,
		/* size  = */ SIZE_64BIT_DATA },

};
extern "C" constexpr Pointer gdt_long_mode_pointer(long_mode);

}  // namespace GDT

#include "machine/fpu.h"
#include "types.h"
#include "machine/core.h"
#include "utils/string.h"

namespace FPU {

static bool has_initial_state = false;
static State initial_state;

void State::init() {
	assert(has_initial_state && "FPU has not been initialized!");
	memcpy(this, &initial_state, sizeof(State));
}

bool init() {
	// Unset bits EM (2, Software Emulation) & TS (3, Task Switched), and set bit MP (1, Monitor Coprocessor) in CR0
	// using the common read-modify-write cycle.
	uintptr_t cr0_value = Core::CR<0>::read();
	cr0_value &= ~(Core::CR0_EM | Core::CR0_TS);
	cr0_value |= Core::CR0_MP;
	Core::CR<0>::write(cr0_value);

	// Initialize FPU, read state and control
	uint16_t status;
	uint16_t control;
	asm volatile(
		"fninit\n\t"
		"fnstsw %0\n\t"
		"fnstcw %1\n\t"
		: "+m"(status), "+m"(control)
	);
	// state must be 0 and the correct bits have to be set in control
	if (status != 0 || (control & 0x103f) != 0x3f) {
		return false;
	}

	// Set bits OSFXSR (9, Enable SSE Instructions) and OSXMMEXCPT (10, Enable SSE Exceptions / Interrupt 19) in CR4
	uintptr_t cr4_value = Core::CR<4>::read();
	cr4_value |= Core::CR4_OSFXSR | Core::CR4_OSXMMEXCPT;
	Core::CR<4>::write(cr4_value);

	// Store initial state
	if (!__atomic_test_and_set(&has_initial_state, __ATOMIC_RELAXED)) {
		initial_state.save();
	}

	// Successfully initialized
	return true;
}
}  // namespace FPU

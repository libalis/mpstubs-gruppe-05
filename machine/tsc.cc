#include "tsc.h"
#include "machine/core.h"
#include "machine/cpuid.h"
#include "machine/pit.h"
#include "debug/output.h"

namespace TSC {
/*! \brief Calculate the TSC frequency, if possible
 *  \note Does not work in QEMU
 *  \return Ticks per millisecond, or 0 if not possible
 */
static uint32_t ticksByProcessorInfo(void) {
	// Check if Intel processor (others are currently not supported)
	CPUID::Reg r = CPUID::get(CPUID::MANUFACTURER_ID);
	// For Intel CPUs, ebx, edx, and ecx contain the string "Genu", "ineI", ¨ntel"
	if (r.ebx != 0x756e6547 || r.edx != 0x49656e69 || r.ecx != 0x6c65746e) {
		return 0;
	}

	// get CPUID
	r = CPUID::get(CPUID::PROCESSOR_INFO);

	// Required features: Both Timestamp Counter (TSC), as well as the Model Specific Registers (MSRs)
	if ((r.edx & CPUID::FEATURE_TSC) == 0 || (r.edx & CPUID::FEATURE_MSR) == 0) {
		return 0;
	}

	// MSR 0xCE contains the Maximum Non-Turbo Ratio in bits 8-15
	uint64_t ratio = (Core::MSR<Core::MSR_PLATFORM_INFO>::read() & 0xff00) >> 8;
	// Check if ratio from MSR contains a reasonable value (check needed for QEMU, for instance)
	if (ratio == 0) {
		return 0;
	}

	// Get Intel Prozessor Model
	int model = ((r.eax & 0xf0) >> 4) | ((r.eax & 0xf0000) >> 12);

	// The base frequency depends on the CPU Model (see Intel Manual Vol 4)
	uint64_t hz;
	switch (model) {
		// Nehalem and Westmere have a base frequency of 133 1/3 MHz
		case 0x1a:
		case 0x1e:
		case 0x1f:
		case 0x2e:
		case 0x25:
		case 0x2c:
		case 0x2f:
			hz = 133333333;
			break;

		// Sandy Bridge, Ivy Bridge, Haswell, Broadwell, and Xeon Phi have a base frequency of 100 MHz
		case 0x2a:
		case 0x2d:
		case 0x3a:
		case 0x3e:
		case 0x3c:
		case 0x3f:
		case 0x45:
		case 0x46:
		case 0x3d:
		case 0x47:
		case 0x4f:
		case 0x56:
		case 0x57:
			hz = 100000000;
			break;

		// Atom Goldmont has a base frequency of 19.2 MHz
		case 0x5c:
			hz = 19200000;
			break;

		// Skylake/Kaby Lake Mobile/Desktop have a base frequency of 24 MHz
		case 0x4e:
		case 0x5e:
		case 0x8e:
		case 0x9e:
			hz = 24000000;
			break;

		// Skylake X and Atom Denverton: 25 MHz
		case 0x55:
		case 0x5f:
			hz = 25000000;
			break;

		// CPU unknown, abort
		default:
			return 0;
	}

	// Derive ticks per millisecond from base frequency
	return (ratio * hz) / 1000;
}

/*! \brief Measure the TSC frequency using PIT
 * \return TSC ticks per millisecond
 */
static uint32_t ticksByPIT(void) {
	uint64_t ticks = 0;
	// Implementation optional
	return ticks;
}

// Ticks per milliseconds
static uint32_t ticks_value = 0;

uint32_t ticks(bool use_pit) {
	// Check processor info, at first
	if (!use_pit) {
		ticks_value = ticksByProcessorInfo();
	}

	// Alternatively: Use PIT for calibration
	// Take care: Not yet implemented; optional
	if (ticks_value == 0 || use_pit) {
		ticks_value = ticksByPIT();
	}

	// Error on invalid value
	if (ticks_value == 0) {
		DBG_VERBOSE << "TSC calibration resulted in ticks = 0" << endl;
		Core::die();
	}

	return ticks_value;
}

bool available(const Instruction instruction) {
	if (instruction == RDTSCP || instruction == RDTSCP_CPUID) {
		return CPUID::has(CPUID::EXTENDED_FEATURE_RDTSCP);
	} else {
		return CPUID::has(CPUID::FEATURE_TSC);
	}
}

uint64_t nanoseconds(uint64_t delta) {
	(void) delta;
	// Implementation optional
	return 0;
}

void delay(uint64_t us) {
	(void) us;
	// Implementation optional
}

}  // namespace TSC

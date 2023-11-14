/*! \file
 *  \brief Helper for floating point unit (x87), Multi Media Extension (MMX)
 *         and Streaming SIMD Extensions (SSE)
 */
#pragma once

#include "types.h"
#include "debug/assert.h"

/*! \brief For the sake of simplicity we call it just FPU, although this is for MMX and SSE as well
 */
namespace FPU {

/*! \brief 512 byte structure for FPU / MMX / SSE registers
 * corresponds to the layout used by the instructions `fxsave` and `fxrstor`.
 * \see [ISDMv1 Chapter 10.5.1 FXSAVE Area](intel_manual_vol1.pdf#page=252)
 */
struct alignas(16) State {
	uint16_t fcw;  ///< FPU Control Word
	uint16_t fsw;  ///< FPU Status Word
	uint8_t ftw;   ///< FPU Tag Word
	uint16_t : 0;  ///< (16 bit alignment)
	uint16_t fop;  ///< FPU Opcode

	// FPU Instruction Pointer
	union {
		uint64_t ip;
		struct {
			uint32_t ip_off;
			uint32_t ip_seg;
		} __attribute__((packed));
	};

	// FPU Data Pointer Offset
	union {
		uint64_t dp;
		struct {
			uint32_t dp_off;
			uint32_t dp_seg;
		} __attribute__((packed));
	};

	// MXCSR Control and Status
	uint32_t mxcsr;
	uint32_t mxcsr_mask;

	/*! \brief 80 bit registers for FPU stack / MMX
	 */
	struct ST {
		uint64_t fraction : 63,
		         integer : 1;
		uint16_t exponent : 15,
		         sign : 1;
	} __attribute__((packed));
	struct {
		ST value;
		uint64_t : 48;  // (alignment to 64 bit, which leads to a total of 128 Bit)
	} st[8];

	/*! \brief 128 bit SSE registers
	 */
	struct XMM {
		uint64_t low;
		uint64_t high;
	} __attribute__((packed));
	XMM xmm[16];  // SSE State

	uint64_t reserved[12];  ///< (reserved)

	/*! \brief Set structure to the initial FPU state
	 * Clears the FPU state (but its not just zeroed!)
	 */
	void init();

	/*! \brief Save the current FPU (+ MMX + SSE) state into this structure
	 * \note Structure has to be 16 byte aligned
	 */
	inline void save() {
		assert(reinterpret_cast<uintptr_t>(this) % 16 == 0);
		asm volatile("fxsave %0\n\t" : : "m"(*this));
	}

	/*! \brief Restore the saved FPU (+ MMX + SSE) state from this structure
	 * \note Structure has to be 16 byte aligned
	 */
	inline void restore() {
		assert(reinterpret_cast<uintptr_t>(this) % 16 == 0);
		asm volatile("fxrstor %0\n\t" : : "m"(*this));
	}
}  __attribute__((packed));

assert_size(State, 512);

/*! \brief Initialize FPU on the current CPU
 * \return `true` on successful initialization
 */
bool init();
}  // namespace FPU

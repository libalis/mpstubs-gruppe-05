/*! \file
 *  \brief \ref Core::MSRs "Identifiers" for \ref Core::MSR "Model-Specific Register"
 */

#pragma once

namespace Core {
/*! \brief Model-Specific Register Identifiers
 *
 * Selection of useful identifiers.
 *
 * \see [ISDMv4](intel_manual_vol4.pdf)
 */
enum MSRs : uint32_t {
	MSR_PLATFORM_INFO = 0xceU,   ///< Platform information including bus frequency (Intel)
	MSR_TSC_DEADLINE  = 0x6e0U,  ///< Register for \ref LAPIC::Timer Deadline mode
	// Fast system calls
	MSR_EFER   = 0xC0000080U,  ///< Extended Feature Enable Register, \see Core::MSR_EFER
	MSR_STAR   = 0xC0000081U,  ///< eip (protected mode), ring 0 and 3 segment bases
	MSR_LSTAR  = 0xC0000082U,  ///< rip (long mode)
	MSR_SFMASK = 0xC0000084U,  ///< lower 32 bit: flag mask, if bit is set corresponding rflag is cleared through syscall

	// Core local variables
	MSR_FS_BASE        = 0xC0000100U,
	MSR_GS_BASE        = 0xC0000101U,  ///< Current GS base pointer
	MSR_SHADOW_GS_BASE = 0xC0000102U,  ///< Usually called `MSR_KERNEL_GS_BASE` but this is misleading
};

/* \brief Important bits in Extended Feature Enable Register (EFER)
 *
 * \see [ISDMv3, 2.2.1 Extended Feature Enable Register](intel_manual_vol3.pdf#page=69)
 * \see [AAPMv2, 3.1.7 Extended Feature Enable Register](amd64_manual_vol2.pdf#page=107)
 */
enum MSR_EFER : uintptr_t {
	MSR_EFER_SCE   = 1U << 0,   ///< System Call Extensions
	MSR_EFER_LME   = 1U << 8,   ///< Long mode enable
	MSR_EFER_LMA   = 1U << 10,  ///< Long mode active
	MSR_EFER_NXE   = 1U << 11,  ///< No-Execute Enable
	MSR_EFER_SVME  = 1U << 12,  ///< Secure Virtual Machine Enable
	MSR_EFER_LMSLE = 1U << 13,  ///< Long Mode Segment Limit Enable
	MSR_EFER_FFXSR = 1U << 14,  ///< Fast `FXSAVE`/`FXRSTOR` instruction
	MSR_EFER_TCE   = 1U << 15,  ///< Translation Cache Extension
};

/*! \brief Access to the Model-Specific Register (MSR)
 *
 * \see [ISDMv3, 9.4 Model-Specific Registers (MSRs)](intel_manual_vol3.pdf#page=319)
 * \see [ISDMv4](intel_manual_vol4.pdf)
 * \tparam id ID of the Model-Specific Register to access
 */
template<enum MSRs id>
class MSR {
	/*! \brief Helper to access low and high bits of a 64 bit value
	 * \internal
	 */
	union uint64_parts {
		struct {
			uint32_t low;
			uint32_t high;
		} __attribute__((packed));
		uint64_t value;

		explicit uint64_parts(uint64_t value = 0) : value(value) {}
	};

 public:
	/*! \brief Read the value of the current MSR
	 *
	 * \return Value stored in the MSR
	 *
	 * \see [ISDMv2, Chapter 4. RDMSR - Read from Model Specific Register](intel_manual_vol2.pdf#page=1186)
	 */
	static inline uint64_t read() {
		uint64_parts p;
		asm volatile ("rdmsr \n\t" : "=a"(p.low), "=d"(p.high) : "c"(id));
		return p.value;
	}

	/*! \brief Write a value into the current MSR
	 *
	 * \param value Value to write into the MSR
	 *
	 * \see [ISDMv2, Chapter 5. WRMSR - Write to Model Specific Register](intel_manual_vol2.pdf#page=1912)
	 */
	static inline void write(uint64_t value) {
		uint64_parts p(value);
		asm volatile ("wrmsr \n\t" : : "c"(id), "a"(p.low), "d"(p.high));
	}
};

}  // namespace Core

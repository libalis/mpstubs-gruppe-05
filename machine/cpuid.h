/*! \file
 * \brief \ref CPUID queries information about the processor
 */
#pragma once

#include "types.h"

/*! \brief Query information about the processor
 *
 * \note This is an interface to the `cpuid` instruction, which can return information
 * about the processor. It should therefor **not** be confused with functionality to
 * \ref Core::getID() "retrieve the ID of the current CPU (core)"!
 */
namespace CPUID {

/*! \brief Structure for register values returned by `cpuid` instruction
 */
union Reg {
	struct {
		uint32_t ebx, edx, ecx, eax;
	};
	char value[16];
};

enum Function : uint32_t {
	HIGHEST_FUNCTION_PARAMETER = 0x0U,  ///< Maximum Input Value for Basic CPUID Information (in register `eax`)
	MANUFACTURER_ID            = 0x0U,  ///< CPU String (in register `ebx`, `ecx` and `edx`)
	PROCESSOR_INFO             = 0x1U,  ///< Version Information like Type, Family, Model (in register `eax`)
	FEATURE_BITS               = 0x1U,  ///< Feature Information (in register `ecx` and `edx`)
	CACHE_INFORMATION          = 0x2U,  ///< Cache and TLB Information
	PROCESSOR_SERIAL_NUMBER    = 0x3U,  ///< deprecated
	HIGHEST_EXTENDED_FUNCTION  = 0x80000000U,  ///< Maximum Input Value for Extended Function CPUID (in register `eax`)
	EXTENDED_PROCESSOR_INFO    = 0x80000001U,  ///< Extended Processor Signature and Feature Bits (in register `eax`)
	EXTENDED_FEATURE_BITS      = 0x80000001U,  ///< Extended Feature Information (in register `ecx` and `edx`)
	PROCESSOR_BRAND_STRING_1   = 0x80000002U,  ///< Processor Brand String (1/3)
	PROCESSOR_BRAND_STRING_2   = 0x80000003U,  ///< Processor Brand String (2/3)
	PROCESSOR_BRAND_STRING_3   = 0x80000004U,  ///< Processor Brand String (3/3)
	ADVANCED_POWER_MANAGEMENT  = 0x80000007U,  ///< Advanced Power Management (with Invariant TSC in register `edx`)
	ADDRESS_SIZES              = 0x80000008U,  ///< Linear/Physical Address size (in register `eax`)
};

/*! \brief Get CPU identification and feature information
 *
 * \param eax Requested feature
 * \return Register values filled by instruction `cpuid` for the requested feature
 *
 * \see [ISDMv2, Chapter 3. CPUID - CPU Identification](intel_manual_vol2.pdf#page=292)
 */
inline Reg get(Function eax) {
	Reg r;
	asm volatile("cpuid \n\t" : "=a"(r.eax), "=b"(r.ebx), "=c"(r.ecx), "=d"(r.edx) : "0" (eax));
	return r;
}

enum FeatureECX : uint32_t {
	FEATURE_SSE3         = 1U << 0,   ///< Prescott New Instructions-SSE3 (PNI)
	FEATURE_PCLMUL       = 1U << 1,   ///< Carry-less Multiplication
	FEATURE_DTES64       = 1U << 2,   ///< 64-bit debug store (edx bit 21)
	FEATURE_MONITOR      = 1U << 3,   ///< MONITOR and MWAIT instructions (SSE3)
	FEATURE_DS_CPL       = 1U << 4,   ///< CPL qualified debug store
	FEATURE_VMX          = 1U << 5,   ///< Virtual Machine eXtensions
	FEATURE_SMX          = 1U << 6,   ///< Safer Mode Extensions (LaGrande)
	FEATURE_EST          = 1U << 7,   ///< Enhanced SpeedStep
	FEATURE_TM2          = 1U << 8,   ///< Thermal Monitor 2
	FEATURE_SSSE3        = 1U << 9,   ///< Supplemental SSE3 instructions
	FEATURE_CID          = 1U << 10,  ///< L1 Context ID
	FEATURE_SDBG         = 1U << 11,  ///< Silicon Debug interface
	FEATURE_FMA          = 1U << 12,  ///< Fused multiply-add (FMA3)
	FEATURE_CX16         = 1U << 13,  ///< CMPXCHG16B instruction
	FEATURE_ETPRD        = 1U << 14,  ///< Can disable sending task priority messages
	FEATURE_PDCM         = 1U << 15,  ///< Perfmon & debug capability
	FEATURE_PCIDE        = 1U << 17,  ///< Process context identifiers (CR4 bit 17)
	FEATURE_DCA          = 1U << 18,  ///< Direct cache access for DMA writes
	FEATURE_SSE4_1       = 1U << 19,  ///< SSE4.1 instructions
	FEATURE_SSE4_2       = 1U << 20,  ///< SSE4.2 instructions
	FEATURE_X2APIC       = 1U << 21,  ///< x2APIC
	FEATURE_MOVBE        = 1U << 22,  ///< MOVBE instruction (big-endian)
	FEATURE_POPCNT       = 1U << 23,  ///< POPCNT instruction
	FEATURE_TSC_DEADLINE = 1U << 24,  ///< APIC implements one-shot operation using a TSC deadline value
	FEATURE_AES          = 1U << 25,  ///< AES instruction set
	FEATURE_XSAVE        = 1U << 26,  ///< XSAVE, XRESTOR, XSETBV, XGETBV
	FEATURE_OSXSAVE      = 1U << 27,  ///< XSAVE enabled by OS
	FEATURE_AVX          = 1U << 28,  ///< Advanced Vector Extensions
	FEATURE_F16C         = 1U << 29,  ///< F16C (half-precision) FP feature
	FEATURE_RDRND        = 1U << 30,  ///< RDRAND (on-chip random number generator) feature
	FEATURE_HYPERVISOR   = 1U << 31   ///< Hypervisor present (always zero on physical CPUs)
};

enum FeatureEDX : uint32_t {
	FEATURE_FPU          = 1U << 0,   ///< Onboard x87 FPU
	FEATURE_VME          = 1U << 1,   ///< Virtual 8086 mode extensions (such as VIF, VIP, PIV)
	FEATURE_DE           = 1U << 2,   ///< Debugging extensions (CR4 bit 3)
	FEATURE_PSE          = 1U << 3,   ///< Page Size Extension
	FEATURE_TSC          = 1U << 4,   ///< Time Stamp Counter
	FEATURE_MSR          = 1U << 5,   ///< Model-specific registers
	FEATURE_PAE          = 1U << 6,   ///< Physical Address Extension
	FEATURE_MCE          = 1U << 7,   ///< Machine Check Exception
	FEATURE_CX8          = 1U << 8,   ///< CMPXCHG8 (compare-and-swap) instruction
	FEATURE_APIC         = 1U << 9,   ///< Onboard Advanced Programmable Interrupt Controller
	FEATURE_SEP          = 1U << 11,  ///< SYSENTER and SYSEXIT instructions
	FEATURE_MTRR         = 1U << 12,  ///< Memory Type Range Registers
	FEATURE_PGE          = 1U << 13,  ///< Page Global Enable bit in CR4
	FEATURE_MCA          = 1U << 14,  ///< Machine check architecture
	FEATURE_CMOV         = 1U << 15,  ///< Conditional move and FCMOV instructions
	FEATURE_PAT          = 1U << 16,  ///< Page Attribute Table
	FEATURE_PSE36        = 1U << 17,  ///< 36-bit page size extension
	FEATURE_PSN          = 1U << 18,  ///< Processor Serial Number
	FEATURE_CLF          = 1U << 19,  ///< CLFLUSH instruction (SSE2)
	FEATURE_DTES         = 1U << 21,  ///< Debug store: save trace of executed jumps
	FEATURE_ACPI         = 1U << 22,  ///< Onboard thermal control MSRs for ACPI
	FEATURE_MMX          = 1U << 23,  ///< MMX instructions
	FEATURE_FXSR         = 1U << 24,  ///< FXSAVE, FXRESTOR instructions, CR4 bit 9
	FEATURE_SSE          = 1U << 25,  ///< SSE instructions (a.k.a. Katmai New Instructions)
	FEATURE_SSE2         = 1U << 26,  ///< SSE2 instructions
	FEATURE_SS           = 1U << 27,  ///< CPU cache implements self-snoop
	FEATURE_HTT          = 1U << 28,  ///< Hyper-threading
	FEATURE_TM1          = 1U << 29,  ///< Thermal monitor automatically limits temperature
	FEATURE_IA64         = 1U << 30,  ///< IA64 processor emulating x86
	FEATURE_PBE          = 1U << 31   ///< Pending Break Enable (PBE# pin) wakeup capability
};

enum ExtendedFeatureEDX : uint32_t {
	EXTENDED_FEATURE_FPU      = 1U << 0,   ///< Onboard x87 FPU
	EXTENDED_FEATURE_VME      = 1U << 1,   ///< Virtual 8086 mode extensions (such as VIF, VIP, PIV)
	EXTENDED_FEATURE_DE       = 1U << 2,   ///< Debugging extensions (CR4 bit 3)
	EXTENDED_FEATURE_PSE      = 1U << 3,   ///< Page Size Extension
	EXTENDED_FEATURE_TSC      = 1U << 4,   ///< Time Stamp Counter
	EXTENDED_FEATURE_MSR      = 1U << 5,   ///< Model-specific registers
	EXTENDED_FEATURE_PAE      = 1U << 6,   ///< Physical Address Extension
	EXTENDED_FEATURE_MCE      = 1U << 7,   ///< Machine Check Exception
	EXTENDED_FEATURE_CX8      = 1U << 8,   ///< CMPXCHG8 (compare-and-swap) instruction
	EXTENDED_FEATURE_APIC     = 1U << 9,   ///< Onboard Advanced Programmable Interrupt Controller
	EXTENDED_FEATURE_SYSCALL  = 1U << 11,  ///< SYSCALL and SYSRET instructions
	EXTENDED_FEATURE_MTRR     = 1U << 12,  ///< Memory Type Range Registers
	EXTENDED_FEATURE_PGE      = 1U << 13,  ///< Page Global Enable bit in CR4
	EXTENDED_FEATURE_MCA      = 1U << 14,  ///< Machine check architecture
	EXTENDED_FEATURE_CMOV     = 1U << 15,  ///< Conditional move and FCMOV instructions
	EXTENDED_FEATURE_PAT      = 1U << 16,  ///< Page Attribute Table
	EXTENDED_FEATURE_PSE36    = 1U << 17,  ///< 36-bit page size extension
	EXTENDED_FEATURE_MP       = 1U << 19,  ///< Multiprocessor Capable
	EXTENDED_FEATURE_NX       = 1U << 20,  ///< Non-executable bit
	EXTENDED_FEATURE_MMXEXT   = 1U << 22,  ///< extended MMX instructions
	EXTENDED_FEATURE_MMX      = 1U << 23,  ///< MMX instructions
	EXTENDED_FEATURE_FXSR     = 1U << 24,  ///< FXSAVE, FXRESTOR instructions, CR4 bit 9
	EXTENDED_FEATURE_FXSR_OPT = 1U << 25,  ///< FXSAVE, FXRESTOR optimizations
	EXTENDED_FEATURE_PDPE1GB  = 1U << 26,  ///< Gibibyte Pages
	EXTENDED_FEATURE_RDTSCP   = 1U << 27,  ///< CPU cache implements self-snoop
	EXTENDED_FEATURE_LM       = 1U << 29,  ///< Long Mode (x64)
	EXTENDED_FEATURE_3DNOWEXT = 1U << 30,  ///< Extended 3DNow! instructions
	EXTENDED_FEATURE_3DNOW    = 1U << 31   ///< 3DNow! instructions
};

/*! \brief Check if feature is provided by this system
 *
 * \param feature Feature to test
 * \return `true` if available, `false` otherwise
 */
inline bool has(enum FeatureECX feature) {
	return (get(FEATURE_BITS).ecx & feature) != 0;
}

/*! \brief Check if feature is provided by this system
 *
 * \param feature Feature to test
 * \return `true` if available, `false` otherwise
 */
inline bool has(enum FeatureEDX feature) {
	return (get(FEATURE_BITS).edx & feature) != 0;
}

/*! \brief Check if feature is provided by this system
 *
 * \param feature Extended feature to test
 * \return `true` if available, `false` if either feature or extended features are unavailable
 */
inline bool has(enum ExtendedFeatureEDX feature) {
	return (get(EXTENDED_FEATURE_BITS).edx & feature) != 0;
}
}  // namespace CPUID

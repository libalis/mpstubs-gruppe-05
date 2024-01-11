/*! \file
 *  \brief \ref TSC "TimeStamp Counter (TSC)"
 */
#pragma once

#include "types.h"

/*! \brief Access to the Timestamp Counter (of the current Core)
 *
 * [Intel Manual Vol. 3, 17.17 Time-Stamp Counter](intel_manual_vol3.pdf#page=625)
 */
namespace TSC {
	enum Instruction {
		RDTSC,         ///< TSC Read with possible out-of-order execution
		RDTSCP,        ///< Pseudo serializing
		CPUID_RDTSC,   ///< Serializing (via CPUID) TSC read for begin of benchmark
		RDTSCP_CPUID,  ///< Serializing (via CPUID) TSC read for end of benchmark
		CR0_RDTSC,     ///< Serializing (via CR0) TSC read for end of benchmark if RDTSCP is not supported
	};

	/*! \brief Check availability of TSC
	 *
	 * \param instruction Test for the (unserialized) RDTSC or pseudo serializing RDTSCP instruction
	 * \return true if RDTSC[P] is available
	 */
	bool available(Instruction instruction = RDTSC);

	/*! \brief Reads the current timestamp counter
	 *
	 *  While reading the timestamp counter is quite easy, it is difficult to prevent out-of-order execution.
	 *
	 *  \see How to Benchmark Code Execution Times on Intel® IA-32 and IA-64 Instruction Set Architectures [benchmark_code_execution.pdf]
	 *  \param instruction Instruction(s) used to read timestamp counter
	 *  \return Timestamp in TSC ticks
	 */
	inline uint64_t read(Instruction instruction = RDTSC) {
		// Helper structure
		union {
			struct {
				uint32_t low;
				uint32_t high;
			} __attribute__((packed));
			uint64_t value;
		} tsc;

		switch (instruction) {
			case RDTSC:
				asm volatile("rdtsc\n\t"
				             : "=a"(tsc.low), "=d"(tsc.high)
				            );
				break;
			case RDTSCP:
				asm volatile("rdtscp"
				             : "=a"(tsc.low), "=d"(tsc.high)
				             :
				             : "%rcx"
				            );
				break;
			case CPUID_RDTSC:
				asm volatile("cpuid\n\t"
				             "rdtsc\n\t"
				             : "=a" (tsc.low), "=d" (tsc.high)
				             :
				             : "%rbx", "%rcx"
				            );
				break;
			case CR0_RDTSC:
				asm volatile(
				             "mov %%cr0, %%rax\n\t"
				             "mov %%rax, %%cr0\n\t"
				             "rdtsc\n\t"
				             : "=a" (tsc.low), "=d" (tsc.high)
				            );
				break;
			case RDTSCP_CPUID:
				asm volatile("rdtscp\n\t"
				             "mov %%eax, %0\n\t"
				             "mov %%edx, %1\n\t"
				             "cpuid\n\t"
				             : "=r" (tsc.low), "=r" (tsc.high)
				             :
				             : "%rax", "%rbx", "%rcx", "%rdx"
				            );
				break;
			default:
				tsc.value = 0;
		}
		// Return
		return tsc.value;
	}

	/*! \brief Gather the TSC frequency in ticks per milliseconds
	 *
	 *  \param use_pit Enforces the usage of the PIT if set, otherwise the processor infos are queried, at first.
	 *  \return Number of TSC ticks per milliseconds
	 *
	 *  \opt Implement calibration via PIT (in the local function ticksByPit()).
	 *       For testing, it might be useful to compare the return values with different values of \p use_pit.
	 *       The returned values should be similar :)
	 */
	uint32_t ticks(bool use_pit = false);

	/*! \brief Convert a timestamp delta value to nanoseconds
	 *
	 * \note It is necessary to execute `TSC::ticks()` prior calling this function the first time,
	 *       since it uses the cached TSC frequency value gathered by `ticks()` for the calculation.
	 *
	 *  \param delta Delta between two timestamps
	 *  \return Equivalent time in nanoseconds
	 *
	 *  \opt Implement conversion
	 */
	uint64_t nanoseconds(uint64_t delta);

	/*! \brief Actively wait the provided waiting time
	 *
	 * \note It is necessary to execute `TSC::ticks()` prior calling this function the first time,
	 *       since it uses the cached TSC frequency value gathered by `ticks()` for the calculation.
	 *
	 *  \param us waiting time in microseconds
	 *
	 *  \opt Implement conversion
	 */
	void delay(uint64_t us);
}  // namespace TSC

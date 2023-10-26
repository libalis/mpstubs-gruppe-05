#include "machine/core.h"
#include "machine/apic.h"
#include "machine/lapic.h"

/*! \brief Initial size of CPU core stacks
 *
 * Used during startup in `boot/startup.asm`
 */
extern "C" const unsigned long CPU_CORE_STACK_SIZE = 4096;

/*! \brief Reserved memory for CPU core stacks
 */
alignas(16) static unsigned char cpu_core_stack[Core::MAX * CPU_CORE_STACK_SIZE];

/*! \brief Pointer to stack memory
 *
 * Incremented during startup of each core (bootstrap and application processors) in `boot/startup.asm`
 */
unsigned char * cpu_core_stack_pointer = cpu_core_stack;

namespace Core {

static unsigned cores = 0;     ///< Number of available CPU cores
static unsigned core_id[255];  ///< Lookup table for CPU core IDs with LAPIC ID as index

static unsigned online_cores = 0;    ///< Number of currently online CPU cores
static bool online_core[Core::MAX];  ///< Lookup table for online CPU cores with CPU core ID as index

void init() {
	// Increment number of online CPU cores
	if (__atomic_fetch_add(&online_cores, 1, __ATOMIC_RELAXED) == 0) {
		// Fill Lookup table
		for (unsigned i = 0; i < Core::MAX; i++) {
			uint8_t lapic_id = APIC::getLAPICID(i);
			if (lapic_id < APIC::INVALID_ID) {  // ignore invalid LAPICs
				core_id[lapic_id] = i;
				cores++;
			}
		}
	}

	// Get CPU ID
	uint8_t cpu = getID();

	// initialize local APIC with logical APIC ID
	LAPIC::init(APIC::getLogicalAPICID(cpu));

	// set current CPU online
	online_core[cpu] = true;

}

void exit() {
	// CPU core offline
	online_core[getID()] = false;
	__atomic_fetch_sub(&online_cores, 1, __ATOMIC_RELAXED);
}

unsigned getID() {
	return core_id[LAPIC::getID()];
}

unsigned count() {
	return cores;
}

unsigned countOnline() {
	return online_cores;
}

bool isOnline(uint8_t core_id) {
	return core_id > Core::MAX ? false : online_core[core_id];
}

}  // namespace Core

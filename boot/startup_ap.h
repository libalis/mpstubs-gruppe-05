/*! \file
 * \brief Startup of additional cores, the application processors (APs)
 */

#pragma once

#include "types.h"
#include "compiler/fix.h"

/*! \brief Application Processor Boot
 *
 * Interface to boot the APs
 */
namespace ApplicationProcessor {
	/*! \brief Address (below 1 MiB) to which the setup code gets relocated
	 */
	const uintptr_t RELOCATED_SETUP = 0x40000;

	/*! \brief Relocate the real mode setup code
	 *
	 * The application processors (APs) start in real mode, which means that your setup
	 * code must be placed within the first megabyte -- your operating system resides
	 * currently at a much higher address (16 MiB), so the code has to be copied
	 * down there first.
	 *
	 * Luckily, the code in `setup_ap()` can be relocated by copying -- because it
	 * does not use any absolute addressing (except when jumping to the protected
	 * mode function `startup_ap()`).
	 * The function must be copied to the address of \ref RELOCATED_SETUP (0x40000),
	 * so that the APs can start there.
	 *
	 * The memory section contains a reserved area for the \ref GDT and its descriptor,
	 * which has to be assigned first with the contents of \ref ap_gdt.
	 *
	 * \note You could also tell the linker script to put the code directly
	 *       at the appropriate place, but unfortunately the Qemu multiboot
	 *       implementation (via `-kernel` parameter) can't handle it properly.
	 */
	void relocateSetupCode();

	/*! \brief Boot all application processors
	 *
	 * Performs relocation by calling \ref relocateSetupCode()
	 *
	 * \see [ISDMv3, 8.4.4.2 Typical AP Initialization Sequence](intel_manual_vol3.pdf#page=276)
	 */
	void boot();
}  // namespace ApplicationProcessor

/*! \brief Begin of setup code for application processors
 *
 * The setup code has to switch from real mode (16 bit) to protected mode (32 bit),
 * hence it is written in assembly and must be executed in low memory (< 1 MiB).
 *
 * After kernel start the code is somewhere above 16 MiB (the bootstrap
 * processor was already launched in protected mode by the boot loader).
 * Therefore this symbol is required for relocate the code to the position
 * specified by \ref ApplicationProcessor::RELOCATED_SETUP.
 *
 * Luckily, the `setup_ap` code in `boot/startup_ap.asm` is rather simple and
 * doesn't depend on absolute addressing -- and is therefore relocatable.
 *
 * Relocation is done by the function \ref ApplicationProcessor::relocateSetupCode()
 *
 * The `___SETUP_AP_START__` symbol is defined in the linker script (`compiler/section.ld`)
 */
extern char ___SETUP_AP_START__;

/*! \brief End of startup code for application processors
 *
 * This Symbol is defined in the linker script (`compiler/section.ld`)
 */
extern char ___SETUP_AP_END__;

/*! \brief Memory reserved for a temporary real mode GDT
 * within the relocatable memory area of the setup code
 */
extern char setup_ap_gdt;

/*! \brief Memory reserved for a temporary real mode GDT descriptor
 * within the relocatable memory area of the setup code
 */
extern char setup_ap_gdtd;

/*! \brief Entry point for application processors
 *
 * Unlike the bootstrap processor, the application processors have not been
 * setup by the boot loader -- they start in `Real Mode` (16 bit) and have to be
 * switched manually to `Protected Mode` (32 bit).
 * This is exactly what this real mode function does, handing over control
 * to the (32 bit) function \ref startup_ap()
 *
 * This code is written is assembly (`boot/startup_ap.asm`) and relocated by
 * \ref ApplicationProcessor::relocateSetupCode() during
 * \ref ApplicationProcessor::boot()
 */
extern "C" void setup_ap()
ERROR_ON_CALL("The setup function for application processors shall never be called from your code!");

/*! \brief Startup for application processors
 * \ingroup Startup
 *
 * This function behaves similar to \ref startup_bsp():
 * Initializes stack pointer,
 * switches to long mode
 * and calls the C++ \ref kernel_init function
 */
extern "C" void startup_ap()
ERROR_ON_CALL("The startup function for application processors shall never be called from your code!");

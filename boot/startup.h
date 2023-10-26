/*! \file
 * \brief Startup of the first core, also known as bootstrap processor (BSP)
 */
#pragma once

#include "compiler/fix.h"

/*! \brief Entry point of your kernel
 *
 * \ingroup Startup
 *
 * Executed by boot loader.
 * Stores Pointer to \ref Multiboot information structure,
 * initializes stack pointer,
 * switches to long mode
 * and finally calls the C++ \ref kernel_init function
 */
extern "C" void startup_bsp()
ERROR_ON_CALL("The kernel entry point shall never be called from your code!");

/*! \brief Initializes the C++ environment and detects system components
 *
 * \ingroup Startup
 *
 * The startup code(both for \ref startup_bsp "bootstrap" and \ref startup_ap "application processor")
 * jumps to this high level function. After initialization it will call \ref main()
 *or \ref main_ap() respectively
 */
extern "C" [[noreturn]] void kernel_init()
ERROR_ON_CALL("The kernel init function shall never be called from your code!");

/*! \brief Kernels main function
 *
 * Called after initialization of the system by \ref kernel_init()
 *
 * \note This code will only be executed on the booting CPU (i.e., the one with ID 0).
 */
extern "C" int main();

/*! \brief Entry point for application processors
 *
 * Called after initialization of the system by \ref kernel_init()
 *
 * \note Code in this function will be executed on all APs (i.e., all CPUs except ID 0)
 */
extern "C" int main_ap();

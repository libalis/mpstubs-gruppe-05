/*! \file
 *  \brief Initialization functions for global objects required by the compiler
 */

#pragma once

/*! \brief C StartUp (CSU)
 * required by the compiler and provided by the c standard library
 */
namespace CSU {

/*! \brief Call global constructors and initialization functions
 * (this is usually done by __libc_csu_init)
 */
void initializer();

/*! \brief Call global destructors and finalizer functions
 * (this is usually done by __libc_csu_fini)
 */
void finalizer();

}  // namespace CSU

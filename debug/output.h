/*! \file
 *  \brief Debug macros enabling debug output on a separate window for each core.
 */

#pragma once

/*! \def DBG_VERBOSE
 * \brief An output stream, which is only displayed in the debug window in verbose mode
 *
 * \note If a serial console has been implemented, the output can be redirected
 *       to the serial stream instead (by changing the macro) -- this makes the
 *       (usually) very large output more readable (since it allows scrolling back)
 */
#ifdef VERBOSE
// If VERBOSE is defined, forward everything to \ref DBG
#define DBG_VERBOSE DBG
#else
// Otherwise sent everything to the NullStream (which will simply discard everything)
#define DBG_VERBOSE nullstream
// in this case we have to include the null stream
#include "debug/nullstream.h"
#endif

/*! \def DBG
 *  \brief An output stream, which is displayed in the debug window of the core it was executed on
 *
 * In single core (\OOStuBS) this is just an alias to the debug window object
 * `dout`.
 * However, on a multi core system a debug window for each core is
 * required, therefore `dout` has to be an \ref TextStream object array with the
 * core ID as array index  -- the selection is done via Core::getID()
 *
 */
#define DBG dout[Core::getID()]

#include "device/textstream.h"
#include "machine/core.h"

/*! \brief Debug window
 *
 * Debug output using \ref DBG like
 *      `DBG << "var = " << var << endl`
 * should be displayed in window dedicated to the core it is executed on.
 *
 * While this is quite easy on single core systems like \OOStuBS -- they only
 * require a single \ref TextStream object called `dout` -- multi core systems
 * like \MPStuBS need an object array with one window per core.
 * In the latter case direct list initialization can be used:
 *
 * \code{.cpp}
 *  TextStream dout[Core::MAX]{
 *     {0, 40, 17, 21},   // Debug window for core 0, like TextStream(0, 40, 17, 21)
 *     {40, 80, 17, 21},  // Debug window for core 1, like TextStream(40, 80, 17, 21)
 *     //...
 *   };
 * \endcode
 *
 * The debug windows in should be located right below the normal output window
 * without any overlap and should be able to display at least 3 lines.
 * In \MPStuBS, two windows can be placed side-by-side, having 40 columns each.
 *
 *  \todo Define `dout`
 */
extern TextStream dout[Core::MAX];

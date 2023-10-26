/*! \file
 *  \brief Macro to print an error message and stop the current core.
 */

#pragma once

/*! \def kernelpanic
 *  \brief Print an error message in the debug window and \ref Core::die "stop the current core"
 *  \ingroup debug
 *  \param MSG error message
 */
#define kernelpanic(MSG) \
	do { \
			DBG << "PANIC: '" << (MSG) << "' in " << __func__ \
			    << " @ " << __FILE__ << ":" << __LINE__ \
			    << flush; \
		Core::die(); \
	} while (0)

// The includes are intentionally placed at the end, so the macro can be used inside those included files as well.
#include "debug/output.h"
#include "machine/core.h"

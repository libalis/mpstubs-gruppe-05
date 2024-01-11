/*! \file
 *  \brief \ref GuardedScheduler, a \ref Guarded "guarded" interface for \ref Scheduler
 */

#pragma once

#include "thread/thread.h"
#include "thread/scheduler.h"
#include "interrupt/guarded.h"

/*! \brief \ref Guarded interface to the \ref Scheduler used by user applications.
 *
 * Implements the system call interface for class \ref Scheduler. All methods
 * provided by this class are wrappers for the respective method from the base
 * class, which provide additional synchronization by using the class \ref Guarded.
 */
class GuardedScheduler : private Scheduler {
 public:
};

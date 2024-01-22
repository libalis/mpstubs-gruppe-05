/*! \file
 *  \brief \ref GuardedScheduler, a \ref Guarded "guarded" interface for \ref Scheduler
 */

#pragma once

#include "thread/thread.h"
#include "thread/scheduler.h"

/*! \brief \ref Guarded interface to the \ref Scheduler used by user applications.
 *
 * Implements the system call interface for class \ref Scheduler. All methods
 * provided by this class are wrappers for the respective method from the base
 * class, which provide additional synchronization by using the class \ref Guarded.
 */
class GuardedScheduler : private Scheduler {
 public:
    // Include a thread in scheduling decisions.
    // This method will register a thread for scheduling.
    // It will be appended to the ready queue and dispatched once its time has come.
    // Parameters
    //     that	Thread to be scheduled
    // Note
    //     This method is equal to the correspondent method in base class Scheduler,
    //     with the only difference that the call will be protected by a Guarded object.
    // Todo:
    //     Implement method
    static void ready(Thread* that);

    // (Self-)termination of the calling thread.
    // This method can be used by a thread to exit itself.
    // The calling thread will not be appended to the ready queue; a reschedule will be issued.
    // Note
    //     This method is equal to the correspondent method in base class Scheduler,
    //     with the only difference that the call will be protected by a Guarded object.
    // Todo:
    //     Implement method
    static void exit();

    // Kills the passed thread.
    // This method is used to kill the Thread that.
    // For OOStuBS, it is sufficient to remove that from the ready queue and, thereby,
    // exclude the thread from scheduling.
    // For MPStuBS, a simple removal is not sufficient, as the thread might currently be running on another CPU core.
    // In this case, the thread needs to be marked as dying
    // (a flag checked by resume prior to enqueuing into the ready queue)
    // and the executing CPU core needs to be notified.
    // Note: The thread should be able to kill itself.
    // Todo:
    //     Adapt method (for MPStuBS)
    // Note
    //     This method is equal to the correspondent method in base class Scheduler,
    //     with the only difference that the call will be protected by a Guarded object.
    // Todo:
    //     Implement method
    static void kill(Thread* that);

    // Issue a thread change.
    // This method issues the change of the currently active thread
    // without requiring the calling thread to be aware of the other threads.
    // Scheduling decisions, i.e. which thread will be run next,
    // are made by the scheduler itself with the knowledge of the currently ready threads.
    // The currently active thread is appended to the end of the queue;
    // the first thread in the queue will be activated (to implement the FIFO policy).
    // Note
    //     This method is equal to the correspondent method in base class Scheduler,
    //     with the only difference that the call will be protected by a Guarded object.
    // Todo:
    //     Implement method
    static void resume();
};

#include "thread/idlethread.h"
#include "machine/core.h"
#include "syscall/guarded_scheduler.h"
#include "thread/scheduler.h"

IdleThread idlethread[Core::MAX]{};

void IdleThread::action() {
    while (true) {
        Core::Interrupt::disable();
        if (Scheduler::isEmpty()) {
            Core::idle();
        } else {
            Core::Interrupt::enable();
            GuardedScheduler::resume();
        }
    }
}

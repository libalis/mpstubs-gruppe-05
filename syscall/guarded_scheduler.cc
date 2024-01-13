#include "syscall/guarded_scheduler.h"
#include "interrupt/guarded.h"

void GuardedScheduler::ready(Thread* that) {
    Guarded guard;
    Scheduler::ready(that);
}

void GuardedScheduler::exit() {
    Guarded guard;
    Scheduler::exit();
}

void GuardedScheduler::kill(Thread* that) {
    Guarded guard;
    Scheduler::kill(that);
}

void GuardedScheduler::resume() {
    Guarded guard;
    Scheduler::resume();
}

#include "sync/bell.h"
#include "sync/bellringer.h"
#include "thread/scheduler.h"

void Bell::ring() {
    for (Thread* thread = dequeue(); thread != nullptr; thread = dequeue())
        Scheduler::wakeup(thread);
}

void Bell::sleep(unsigned int ms) {
    Bell bell{ms};
    Bellringer::job(&bell, ms);
    Scheduler::block(&bell);
}

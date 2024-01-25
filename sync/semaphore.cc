#include "sync/semaphore.h"
#include "interrupt/guard.h"
#include "thread/scheduler.h"

void Semaphore::p() {
    if (counter > 0)
        counter--;
    else
        Scheduler::block(this);
}

void Semaphore::v() {
    Thread* first = dequeue();
    if (first != nullptr)
        Scheduler::wakeup(first);
    else
        counter++;
}

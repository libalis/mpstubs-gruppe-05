#include "sync/waitingroom.h"
#include "thread/scheduler.h"

Waitingroom::~Waitingroom() {
    for (Thread* customer = dequeue(); customer != nullptr; customer = dequeue()) {
        Scheduler::wakeup(customer);
    }
}

void Waitingroom::remove(Thread* customer) {
    customer->setWaitingroom(nullptr);
    Queue::remove(customer);
}

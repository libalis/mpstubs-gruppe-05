#include "interrupt/guard.h"
#include "interrupt/gatequeue.h"
#include "machine/core.h"
#include "sync/ticketlock.h"

void Guard::enter() {
    corelock[Core::getID()].lock();
}

void Guard::leave() {
    for (Gate* item = gatequeue.dequeue(); item != nullptr; item = gatequeue.dequeue()) {
        BKL.lock();
        item->epilogue();
        BKL.unlock();
    }
    corelock[Core::getID()].unlock();
}

void Guard::relay(Gate* item) {
    if (!gatequeue.enqueue(item)) return;
    if (!corelock[Core::getID()].locked) {
        enter();
        leave();
    }
}

#include "interrupt/guard.h"
#include "interrupt/gatequeue.h"
#include "machine/core.h"
#include "sync/ticketlock.h"

bool locked[Core::MAX];
Ticketlock BKL{};

void Guard::enter() {
    locked[Core::getID()] = true;
    Core::Interrupt::enable();
    BKL.lock();
}

void Guard::leave() {
    bool wasEnabled = Core::Interrupt::disable();
    for (Gate* item = gatequeue.dequeue(); item != nullptr; item = gatequeue.dequeue()) {
        Core::Interrupt::enable();
        item->epilogue();
        Core::Interrupt::disable();
    }
    locked[Core::getID()] = false;
    BKL.unlock();
    Core::Interrupt::restore(wasEnabled);
}

void Guard::relay(Gate* item) {
    if (!gatequeue.enqueue(item))
        return;
    if (!locked[Core::getID()]) {
        enter();
        Core::Interrupt::disable();
        leave();
    }
}

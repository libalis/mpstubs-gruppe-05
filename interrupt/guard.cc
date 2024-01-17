#include "interrupt/guard.h"
#include "interrupt/gatequeue.h"
#include "machine/core.h"
#include "sync/ticketlock.h"

bool locked[Core::MAX];
Ticketlock BKL{};

void Guard::enter() {
    bool wasEnabled = Core::Interrupt::disable();
    locked[Core::getID()] = true;
    Core::Interrupt::restore(wasEnabled);
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
    bool wasEnabled = Core::Interrupt::disable();
    if (!locked[Core::getID()]) {
        Core::Interrupt::restore(wasEnabled);
        enter();
        Core::Interrupt::disable();
        leave();
    } else {
        Core::Interrupt::restore(wasEnabled);
    }
}

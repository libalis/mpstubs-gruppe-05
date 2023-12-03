#include "interrupt/gatequeue.h"
#include "machine/core.h"

Gate* head[Core::MAX];

bool GateQueue::enqueue(Gate* item)  {
    Core::Interrupt::disable();
    if (head[Core::getID()] == nullptr) {
        head[Core::getID()] = item;
        Core::Interrupt::enable();
        return true;
    }
    Gate* current = head[Core::getID()];
    while (current->next[Core::getID()] != nullptr) {
        if (current == item) {
            Core::Interrupt::enable();
            return false;
        }
        current = current->next[Core::getID()];
    }
    current->next[Core::getID()] = item;
    Core::Interrupt::enable();
    return true;
}

Gate* GateQueue::dequeue() {
    Core::Interrupt::disable();
    Gate* current = head[Core::getID()];
    if (current == nullptr) {
        Core::Interrupt::enable();
        return current;
    }
    Gate* item = current;
    head[Core::getID()] = current->next[Core::getID()];
    Core::Interrupt::enable();
    return item;
}

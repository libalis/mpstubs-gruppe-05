#include "interrupt/gatequeue.h"
#include "machine/core.h"

Gate* head[Core::MAX];

bool GateQueue::enqueue(Gate* item)  {
    if (head[Core::getID()] == nullptr) {
        head[Core::getID()] = item;
        return true;
    }
    Gate* current = head[Core::getID()];
    while (current->next[Core::getID()] != nullptr) {
        if (current == item) {
            return false;
        }
        current = current->next[Core::getID()];
    }
    current->next[Core::getID()] = item;
    return true;
}

Gate* GateQueue::dequeue() {
    Gate* current = head[Core::getID()];
    if (current == nullptr) {
        return current;
    }
    Gate* item = current;
    item->next[Core::getID()] = nullptr;
    head[Core::getID()] = current->next[Core::getID()];
    return item;
}

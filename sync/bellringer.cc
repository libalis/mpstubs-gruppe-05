#include "sync/bellringer.h"

Queue<Bell> Bellringer::queue{};

void Bellringer::check() {
    while ((queue.first()->ms)-- == 0) {
        queue.dequeue()->ring();
    }
}

void Bellringer::job(Bell *bell, unsigned int ms) {
    if (queue.first() == nullptr)
        return queue.insertFirst(bell);
    Queue<Bell>::Iterator iterator = queue.begin();
    if (ms < queue.first()->ms) {
        queue.insertFirst(bell);
        (*iterator)->ms -= ms;
        return;
    }
    Queue<Bell>::Iterator before = queue.begin();
    while (iterator != queue.end()) {
        if ((*iterator)->ms < ms) {
            ms -= (*iterator)->ms;
            before = iterator;
            ++iterator;
        } else {
            queue.insertAfter(*before, bell);
            (*iterator)->ms -= ms;
            return;
        }
    }
}

void Bellringer::cancel(Bell *bell) {
    Queue<Bell>::Iterator iterator = queue.begin();
    while (iterator != queue.end()) {
        if (*iterator == bell)
            break;
    }
    (*(++iterator))->ms += bell->ms;
    queue.remove(bell);
}

bool Bellringer::bellPending() {
    return queue.first()== nullptr ? false : true;
}

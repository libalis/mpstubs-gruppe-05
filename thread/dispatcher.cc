#include "thread/dispatcher.h"

Thread* Dispatcher::life_pointer[Core::MAX]{};

void Dispatcher::go(Thread* first) {
    assert(first != nullptr);
    setActive(first);
    first->go();
}

void Dispatcher::dispatch(Thread* next) {
    assert(next != nullptr);
    Thread* current = active();
    setActive(next);
    current->resume(next);
}

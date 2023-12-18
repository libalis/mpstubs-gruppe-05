#include "thread/dispatcher.h"
#include "machine/context.h"

Thread* Dispatcher::life_pointer[Core::MAX]{};

void Dispatcher::go(Thread* first) {
    setActive(first);
    first->go();
}

void Dispatcher::dispatch(Thread* next) {
    Thread* current = active();
    setActive(next);
    current->resume(next);
}

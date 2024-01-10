#include "thread/scheduler.h"

Queue<Thread> Scheduler::readylist{};

void Scheduler::exit() {
    Thread* thread = readylist.dequeue();
    assert(thread != nullptr);
    dispatch(thread);
}

void Scheduler::kill(Thread* that) {
    readylist.remove(that);
    that->kill_flag = true;
}

void Scheduler::ready(Thread* that) {
    readylist.enqueue(that);
}

void Scheduler::resume() {
    if (!active()->kill_flag)
        readylist.enqueue(active());
    Thread* thread = readylist.dequeue();
    assert(thread != nullptr);
    dispatch(thread);
}

void Scheduler::schedule() {
    Thread* thread = readylist.dequeue();
    assert(thread != nullptr);
    go(thread);
}

#include "thread/scheduler.h"
#include "interrupt/guarded.h"

Queue<Thread> Scheduler::readylist{};

void Scheduler::exit() {
    Guarded guard;
    dispatch(readylist.dequeue());
}

void Scheduler::kill(Thread* that) {
    Guarded guard;
    readylist.remove(that);
    that->kill_flag = true;
}

void Scheduler::ready(Thread* that) {
    Guarded guard;
    readylist.enqueue(that);
}

void Scheduler::resume() {
    Guarded guard;
    readylist.enqueue(active());
    dispatch(readylist.dequeue());
}

void Scheduler::schedule() {
    Guarded guard;
    go(readylist.dequeue());
}

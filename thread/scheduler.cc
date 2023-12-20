#include "thread/scheduler.h"

Queue<Thread> Scheduler::readylist{};

void Scheduler::exit() {
    dispatch(readylist.dequeue());
}

void Scheduler::kill(Thread* that) {
    readylist.remove(that);
    that->kill_flag = true;
}

void Scheduler::ready(Thread* that) {
    readylist.enqueue(that);
}

void Scheduler::resume() {
    readylist.enqueue(active());
    dispatch(readylist.dequeue());
}

void Scheduler::schedule() {
    go(readylist.dequeue());
}

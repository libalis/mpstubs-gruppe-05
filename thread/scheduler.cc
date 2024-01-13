#include "thread/scheduler.h"
#include "machine/apic.h"
#include "machine/lapic.h"

Queue<Thread> Scheduler::readylist{};

void Scheduler::exit() {
    Thread* thread = readylist.dequeue();
    assert(thread != nullptr);
    dispatch(thread);
}

void Scheduler::kill(Thread* that) {
    that->kill_flag = true;
    if (readylist.remove(that) == 0) {
        unsigned cpu;
        if (isActive(that, &cpu)) {
            uint8_t destination = APIC::getLAPICID(cpu);
            LAPIC::IPI::send(destination, Core::Interrupt::ASSASSIN);
        }
    }
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

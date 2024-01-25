#include "thread/scheduler.h"
#include "machine/apic.h"
#include "machine/lapic.h"
#include "sync/waitingroom.h"
#include "thread/idlethread.h"

Queue<Thread> Scheduler::readylist{};

void Scheduler::exit() {
    Thread* thread = readylist.dequeue();
    if (thread == nullptr)
        thread = &idlethread[Core::getID()];
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
    if (thread == nullptr)
        thread = &idlethread[Core::getID()];
    dispatch(thread);
}

void Scheduler::schedule() {
    Thread* thread = readylist.dequeue();
    if (thread == nullptr)
        thread = &idlethread[Core::getID()];
    go(thread);
}

void Scheduler::block(Waitingroom* waitingroom) {
    (*waitingroom).enqueue(active());
    Thread* thread = readylist.dequeue();
    if (thread == nullptr)
        thread = &idlethread[Core::getID()];
    dispatch(thread);
}

void Scheduler::wakeup(Thread* customer) {
    customer->getWaitingroom()->remove(customer);
    readylist.enqueue(customer);
    for (unsigned cpu = 0; cpu < Core::count(); cpu++) {
        uint8_t destination = APIC::getLAPICID(cpu);
        LAPIC::IPI::send(destination, Core::Interrupt::WAKEUP);
    }
}

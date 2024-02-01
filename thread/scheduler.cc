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
    that->setWaitingroom(nullptr);
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
    if (!active()->kill_flag && active() != &idlethread[Core::getID()])
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
    active()->setWaitingroom(waitingroom);
    Thread* thread = readylist.dequeue();
    if (thread == nullptr)
        thread = &idlethread[Core::getID()];
    dispatch(thread);
}

void Scheduler::wakeup(Thread* customer) {
    customer->getWaitingroom()->remove(customer);
    readylist.enqueue(customer);
    LAPIC::IPI::sendOthers(Core::Interrupt::WAKEUP);
}

#include "thread/assassin.h"
#include "interrupt/plugbox.h"
#include "thread/dispatcher.h"
#include "syscall/guarded_scheduler.h"

Assassin assassin{};

void Assassin::hire() {
    Plugbox::assign(Core::Interrupt::Vector::ASSASSIN, this);
}

bool Assassin::prologue() {
    return true;
}

void Assassin::epilogue() {
    if (Dispatcher::active()->kill_flag)
        GuardedScheduler::resume();
}

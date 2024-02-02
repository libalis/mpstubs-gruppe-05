#include "thread/wakeup.h"

WakeUp wakeup{};

void WakeUp::activate() {
    Plugbox::assign(Core::Interrupt::Vector::WAKEUP, this);
}

bool WakeUp::prologue() {
    return false;
}

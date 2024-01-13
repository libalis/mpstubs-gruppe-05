#include "device/watch.h"
#include "interrupt/plugbox.h"
#include "machine/lapic.h"
#include "syscall/guarded_scheduler.h"

uint32_t Watch::ival = 0;
uint8_t Watch::divide = 0;
uint32_t Watch::counter = 0;

Watch watch[Core::MAX]{};

bool Watch::windup(uint32_t us) {
    Plugbox::assign(Core::Interrupt::TIMER, this);
    uint32_t ticks = LAPIC::Timer::ticks();
    for (divide = 1; divide < UINT8_MAX; divide *= 2) {
        uint64_t divisor_long = (us * ticks);
        uint32_t divisor = (us * ticks);
        if (divisor_long == divisor) break;
    }
    if (divide == UINT8_MAX)
        return false;
    ival = us;
    counter = (us*ticks)/(divide*1000);
    return true;
}

bool Watch::prologue() {
    return true;
}

void Watch::epilogue() {
    GuardedScheduler::resume();
}

uint32_t Watch::interval() const {
    return ival;
}

void Watch::activate() const {
    LAPIC::Timer::set(counter, divide, Core::Interrupt::TIMER, true);
}

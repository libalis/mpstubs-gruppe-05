#include "device/watch.h"
#include "debug/output.h"
#include "interrupt/plugbox.h"
#include "machine/lapic.h"
#include "sync/bellringer.h"
#include "thread/scheduler.h"

Watch watch{};

bool Watch::windup(uint32_t us) {
    Plugbox::assign(Core::Interrupt::TIMER, this);
    uint64_t u = static_cast<uint64_t>(us) * LAPIC::Timer::ticks();
    u /= 1000;
    uint16_t div;
    for (div = 1; div < UINT8_MAX; div *= 2) {
        uint64_t divisor_long = u / div;
        uint32_t divisor = u / div;
        if (divisor_long == divisor) break;
    }
    if (div >= UINT8_MAX)
        return false;
    divide = div;
    ival = us;
    counter = u / divide;
    return true;
}

bool Watch::prologue() {
    static int i = 0;
    DBG << "TIMER " << i++ << endl;
    return true;
}

void Watch::epilogue() {
    if (Core::getID() == 0)
        Bellringer::check();
    Scheduler::resume();
}

uint32_t Watch::interval() const {
    return ival;
}

void Watch::activate() const {
    LAPIC::Timer::set(counter, divide, Core::Interrupt::TIMER, true);
}

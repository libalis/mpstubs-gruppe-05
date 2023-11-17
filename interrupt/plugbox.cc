#include "interrupt/plugbox.h"
#include "device/keyboard.h"
#include "device/panic.h"
#include "machine/core_interrupt.h"

Gate* plugbox[Core::Interrupt::VECTORS]{};
Panic* panic{};

void Plugbox::assign(Core::Interrupt::Vector vector, Gate *gate) {
    plugbox[vector] = gate;
}

Gate* Plugbox::report(Core::Interrupt::Vector vector) {
    return plugbox[vector] == NULL ? panic : plugbox[vector];
}

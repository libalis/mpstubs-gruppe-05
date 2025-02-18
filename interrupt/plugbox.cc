#include "interrupt/plugbox.h"
#include "device/panic.h"
#include "machine/core_interrupt.h"

Gate* plugbox[Core::Interrupt::VECTORS];
Panic panic{};

void Plugbox::assign(Core::Interrupt::Vector vector, Gate *gate) {
    plugbox[vector] = gate;
}

Gate* Plugbox::report(Core::Interrupt::Vector vector) {
    return (plugbox[vector] != nullptr) ? plugbox[vector] : &panic;
}

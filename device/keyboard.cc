#include "device/keyboard.h"
#include "debug/output.h"
#include "interrupt/plugbox.h"
#include "machine/apic.h"
#include "machine/core_interrupt.h"
#include "machine/ioapic.h"
#include "machine/ps2controller.h"
#include "machine/system.h"

Keyboard keyboard{};

void Keyboard::plugin() {
    PS2Controller::init();
    PS2Controller::drainBuffer();
    Plugbox::assign(Core::Interrupt::KEYBOARD, this);
    uint8_t slot = APIC::getIOAPICSlot(APIC::Device::KEYBOARD);
    IOAPIC::config(slot, Core::Interrupt::KEYBOARD, IOAPIC::LEVEL);
    IOAPIC::allow(slot);
}

bool Keyboard::prologue() {
    Key input;
    if (PS2Controller::fetch(input)) {
        if (input.ctrl() && input.alt() && input.scancode == Key::KEY_DEL)
            System::reboot();
        return pro.produce(input);
    }
    return false;
}

void Keyboard::epilogue() {
    Key k;
    while (pro.consume(k)) {
        if (epi.produce(k))
            semaphore.v();
    }
}

Key Keyboard::getKey() {
    semaphore.p();
    Key k;
    bool b = epi.consume(k);
    assert(b);
    return k;
}

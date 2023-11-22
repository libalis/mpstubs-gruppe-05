#include "device/keyboard.h"
#include "debug/output.h"
#include "interrupt/plugbox.h"
#include "machine/apic.h"
#include "machine/core_interrupt.h"
#include "machine/ioapic.h"
#include "machine/ps2controller.h"
#include "machine/system.h"
#include "sync/ticketlock.h"

void Keyboard::plugin() {
    Plugbox::assign(Core::Interrupt::KEYBOARD, this);
    uint8_t slot = APIC::getIOAPICSlot(APIC::Device::KEYBOARD);
    IOAPIC::config(slot, Core::Interrupt::KEYBOARD, IOAPIC::LEVEL);
    IOAPIC::allow(slot);
}

void Keyboard::trigger() {
    Key pressed;
    if (PS2Controller::fetch(pressed)) {
        ticketlock.lock();
        kout.setPos(position, 0);
        position++;
        position %= TextMode::COLUMNS;
        kout << pressed.ascii();
        kout.flush();
        ticketlock.unlock();
    }
    if (pressed.ctrl() && pressed.alt() && pressed.scancode == Key::KEY_DEL) System::reboot();
}

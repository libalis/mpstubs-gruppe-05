#include "device/keyboard.h"
#include "debug/output.h"
#include "interrupt/plugbox.h"
#include "machine/apic.h"
#include "machine/core_interrupt.h"
#include "machine/ioapic.h"
#include "machine/ps2controller.h"
#include "machine/system.h"

void Keyboard::plugin() {
    Plugbox::assign(Core::Interrupt::KEYBOARD, this);
    uint8_t slot = APIC::getIOAPICSlot(APIC::Device::KEYBOARD);
    IOAPIC::config(slot, Core::Interrupt::KEYBOARD, IOAPIC::LEVEL);
    IOAPIC::allow(slot);
}

bool Keyboard::prologue() {
    return PS2Controller::fetch(pressed);
}

void Keyboard::epilogue() {
    if (pressed.ctrl() && pressed.alt() && pressed.scancode == Key::KEY_DEL)
        System::reboot();
    if (pressed.scancode == Key::KEY_BACKSPACE) {
        if (position != 0)
            position--;
        kout.setPos(position, 0);
        kout << ' ';
        kout.flush();
    } else if (pressed.scancode == Key::KEY_ENTER) {
        for (position = 0; position < TextMode::COLUMNS; position++) {
            kout.setPos(position, 0);
            kout << ' ';
            kout.flush();
        }
        position = 0;
    } else {
        kout.setPos(position, 0);
        position++;
        position %= TextMode::COLUMNS;
        kout << pressed.ascii();
        kout.flush();
    }
}

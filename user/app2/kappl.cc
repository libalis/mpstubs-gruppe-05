#include "user/app2/kappl.h"
#include "debug/output.h"
#include "device/keyboard.h"
#include "interrupt/guarded.h"

void KeyboardApplication::action() {
    Key pressed;
    uint8_t position = 0;
    while (true) {
        Guarded guard;
        pressed = keyboard.getKey();
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
}

#include "user/app2/kappl.h"
#include "machine/ps2controller.h"

void KeyboardApplication::action() {
    PS2Controller::init();
    Key pressed;
    while (true) {
        PS2Controller::fetch(pressed);
        kout << pressed.ascii();
    }
}

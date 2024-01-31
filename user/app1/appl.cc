#include "user/app1/appl.h"
#include "debug/output.h"
#include "interrupt/guarded.h"
#include "machine/core.h"
#include "machine/pit.h"
#include "sync/bell.h"
#include "thread/scheduler.h"

extern Application app[];

void Application::action() {
    if (id == 8) {
        while (true) {
            Guarded guard;
            static const int melody[][2] = {
                {659, 120}, {622, 120}, {659, 120}, {622, 120}, {659, 120}, {494, 120},
                {587, 120}, {523, 120}, {440, 120}, {262, 120}, {330, 120}, {440, 120},
                {494, 120}, {330, 120}, {415, 120}, {494, 120}, {523, 120}, {330, 120},
                {659, 120}, {622, 120}, {659, 120}, {622, 120}, {659, 120}, {494, 120},
                {587, 120}, {523, 120}, {440, 120}, {262, 120}, {330, 120}, {440, 120},
                {494, 120}, {330, 120}, {523, 120}, {494, 120}, {440, 120}, {0, 10}
            };
            kout.setPos(0, id + 1);
            kout << static_cast<char>(14) << flush;
            for (auto tone : melody) {
                Core::Interrupt::disable();
                PIT::pcspeaker(tone[0]);
                Core::Interrupt::enable();
                Bell::sleep(tone[1]);
            }
            kout.setPos(0, id + 1);
            kout << "." << endl;
        }
    }
    uint64_t count = 0;
    while (true) {
        Guarded guard;
        kout.setPos(0, id + 1);
        kout << count++;
        kout.flush();
        if (id == 2 && count >= 10000) Scheduler::kill(&app[0]);
        if (id == 1 && count >= 10000) Scheduler::kill(this);
    }
}

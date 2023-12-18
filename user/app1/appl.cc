#include "user/app1/appl.h"
#include "debug/output.h"
#include "interrupt/guarded.h"
#include "machine/core.h"
#include "thread/scheduler.h"

void Application::action() {
    uint64_t count = 0;
    while (true) {
        Guarded section;
        kout.setPos(0, id + 1);
        kout << count++;
        kout.flush();
        Scheduler::resume();
    }
}

#include "user/app1/appl.h"
#include "debug/output.h"
#include "interrupt/guarded.h"
#include "syscall/guarded_scheduler.h"

extern Application app[];

void Application::action() {
    uint64_t count = 0;
    while (true) {
        Guarded section;
        kout.setPos(0, id + 1);
        kout << count++;
        kout.flush();
        if (id == 2 && count >= 10000) GuardedScheduler::kill(&app[0]);
        if (id == 1 && count >= 10000) GuardedScheduler::kill(this);
    }
}

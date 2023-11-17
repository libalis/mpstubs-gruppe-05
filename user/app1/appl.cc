#include "user/app1/appl.h"
#include "debug/output.h"
#include "machine/core.h"
#include "sync/ticketlock.h"

void Application::action() {
    uint64_t count = 0;
    while (true) {
        koutLock.lock();
        kout.setPos(0, Core::getID() + 1);
        kout << count++;
        kout.flush();
        koutLock.unlock();
    }
}

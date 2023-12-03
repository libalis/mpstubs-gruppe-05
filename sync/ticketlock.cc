#include "sync/ticketlock.h"

void Ticketlock::lock() {
    uint64_t ticket = __atomic_fetch_add(&ticket_count, 1, __ATOMIC_SEQ_CST);
    while (ticket != ticket_current) {
        Core::pause();
    }
    locked = true;
}

void Ticketlock::unlock() {
    locked = false;
    __atomic_fetch_add(&ticket_current, 1, __ATOMIC_SEQ_CST);
}

#include "device/panic.h"
#include "debug/output.h"

void Panic::trigger() {
    DBG << "Panic::trigger()" << endl;
    Core::die();
}

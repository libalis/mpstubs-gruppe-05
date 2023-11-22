#include "device/panic.h"
#include "debug/output.h"

void Panic::trigger() {
    DBG << "Panic::trigger";
    DBG.flush();
    Core::die();
}

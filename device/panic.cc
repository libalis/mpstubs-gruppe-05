#include "device/panic.h"
#include "debug/output.h"

bool Panic::prologue() {
    DBG << "Panic::trigger";
    DBG.flush();
    Core::die();
    return false;
}

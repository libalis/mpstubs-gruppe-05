#include "machine/system.h"
#include "machine/cmos.h"
#include "machine/ioport.h"
#include "debug/output.h"

namespace System {

void reboot() {
	const IOPort system_control_port_a(0x92);
	DBG_VERBOSE << "rebooting smp" << endl;
	CMOS::write(CMOS::REG_STATUS_SHUTDOWN, 0);
	system_control_port_a.outb(0x3);
}

}  // namespace System

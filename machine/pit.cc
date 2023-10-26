#include "machine/pit.h"
#include "machine/ioport.h"
#include "machine/core.h"

namespace PIT {

// we only use PIT channel 2
const uint8_t CHANNEL = 2;
static IOPort data(0x40 + CHANNEL);

/*! \brief Access mode
 */
enum AccessMode {
	LATCH_COUNT_VALUE  = 0,
	LOW_BYTE_ONLY      = 1,
	HIGH_BYTE_ONLY     = 2,
	LOW_AND_HIGH_BYTE  = 3
};

/*! \brief Operating Mode
 *
 * \warning Channel 2 is not able to send interrupts, however, the status bit will be set
 */
enum OperatingMode {
	INTERRUPT_ON_TERMINAL_COUNT = 0,
	PROGRAMMABLE_ONE_SHOT       = 1,
	RATE_GENERATOR              = 2,
	SQUARE_WAVE_GENERATOR       = 3,  ///< useful for the PC speaker
	SOFTWARE_TRIGGERED_STROBE   = 4,
	HARDWARE_TRIGGERED_STROBE   = 5
};

/*! \brief data format
 */
enum Format {
	BINARY = 0,
	BCD    = 1   ///< Binary Coded Decimals
};

// Mode register (only writable)
static IOPort mode_register(0x43);
union Mode {
	struct {
		Format format           : 1;
		OperatingMode operating : 3;
		AccessMode access       : 2;
		uint8_t channel         : 2;
	};
	uint8_t value;

	/*! \brief Constructor for mode, takes the numeric value */
	explicit Mode(uint8_t value) : value(value) {}

	/*! \brief Constructor for counting mode
	 *  \param access    Access mode to the 16-bit counter value
	 *  \param operating Operating mode for the counter
	 *  \param format    Number format for the 16-bit counter values (binary or BCD)
	 */
	Mode(AccessMode access, OperatingMode operating, Format format) :
		format(format), operating(operating), access(access), channel(PIT::CHANNEL) {}

	/*! \brief (Default) constructor for reading the counter value
	 */
	Mode() : value(0) {
		this->channel = PIT::CHANNEL;
	}

	/*! \brief Write the value to the mode register
	 */
	void write() const {
		mode_register.outb(value);
	}
};

// The NMI Status and Control Register contains details about PIT counter 2
static IOPort controlRegister(0x61);
union Control {
	/*! \brief I/O-port bitmap for the NMI Status and Control Register
	 *  \note Over time, the meaning of the bits stored at I/O port 0x61 changed; don't get the structure confused
	 *        with old documentation on the IBM PC XT platform.
	 *  \see [Intel® I/O Controller Hub 7 (ICH7) Family](i-o-controller-hub-7-datasheet.pdf#page=415), page 415
	 */
	struct {
		//! If enabled, the interrupt state will be visible at status_timer_counter2
		uint8_t enable_timer_counter2         : 1;
		uint8_t enable_speaker_data           : 1;  ///< If set, speaker output is equal to status_timer_counter2
		uint8_t enable_pci_serr               : 1;  ///< not important, do not modify
		uint8_t enable_nmi_iochk              : 1;  ///< not important, do not modify
		const uint8_t refresh_cycle_toggle    : 1;  ///< not important, must be 0 on write
		const uint8_t status_timer_counter2   : 1;  ///< will be set on timer expiration; must be 0 on write
		const uint8_t status_iochk_nmi_source : 1;  ///< not important, must be 0 on write
		const uint8_t status_serr_nmi_source  : 1;  ///< not important, must be 0 on write
	};
	uint8_t value;

	/*! \brief Constructor
	 *  \param value Numeric value for the control register
	 */
	explicit Control(uint8_t value) : value(value) {}

	/*! \brief Default constructor
	 *  Automatically reads the current contents from the control register.
	 */
	Control() : value(controlRegister.inb()) {}

	/*! \brief Write the current state to the control register.
	 */
	void write() const {
		controlRegister.outb(value);
	}
};

// The base frequency is, due to historic reasons, 1.193182 MHz.
const uint64_t BASE_FREQUENCY = 1193182ULL;

bool set(uint16_t us) {
	// Counter ticks for us
	uint64_t counter = BASE_FREQUENCY * us / 1000000ULL;

	// As the hardware counter has a size of 16 bit, we want to check whether the
	// calculated counter value is too large ( > 54.9ms )
	if (counter > 0xffff) {
		return false;
	}

	// Interrupt state should be readable in status register, but PC speaker should remain off
	Control c;
	c.enable_speaker_data = 0;
	c.enable_timer_counter2 = 1;
	c.write();

	// Channel 2, 16-bit divisor, with mode 0 (interrupt) in binary format
	Mode m(AccessMode::LOW_AND_HIGH_BYTE, OperatingMode::INTERRUPT_ON_TERMINAL_COUNT, Format::BINARY);
	m.write();

	// Set the counter's start value
	data.outb(counter & 0xff);         // low
	data.outb((counter >> 8) & 0xff);  // high

	return true;
}

uint16_t get(void) {
	// Set mode to reading
	Mode m;
	m.write();

	// Read low and high
	uint16_t value = data.inb();
	value |= data.inb() << 8;

	return value;
}

bool isActive(void) {
	Control c;  // reads the current value from the control register
	return c.enable_timer_counter2 == 1 && c.status_timer_counter2 == 0;
}

bool waitForTimeout(void) {
	while(true) {
		Control c;  // reads the current value from the control register
		if (c.enable_timer_counter2 == 0) {
			return false;
		} else if (c.status_timer_counter2 == 1) {
			return true;
		} else {
			Core::pause();
		}
	}
}

bool delay(uint16_t us) {
	return set(us) && waitForTimeout();
}

void pcspeaker(uint32_t freq) {
	Control c;
	if (freq == 0) {
		disable();
	} else {
		// calculate frequency divider
		uint64_t div = BASE_FREQUENCY / freq;
		if (div > 0xffff) {
			div = 0xffff;
		}

		// check if already configured
		if (c.enable_speaker_data == 0) {
			// if not, set mode
			Mode m(AccessMode::LOW_AND_HIGH_BYTE, OperatingMode::SQUARE_WAVE_GENERATOR, Format::BINARY);
			m.write();
		}

		// write frequency divider
		data.outb(div & 0xff);
		data.outb((div >> 8) & 0xff);

		// already configured? (second part to prevent playing a wrong sound)
		if (c.enable_speaker_data == 0) {
			// activate PC speaker
			c.enable_speaker_data = 1;
			c.enable_timer_counter2 = 1;
			c.write();
		}
	}
}

void disable(void) {
	Control c;
	c.enable_speaker_data = 0;
	c.enable_timer_counter2 = 0;
	c.write();
}

}  // namespace PIT

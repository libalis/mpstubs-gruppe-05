#include "types.h"
#include "machine/lapic.h"
#include "machine/lapic_registers.h"
#include "machine/core.h"
#include "machine/pit.h"
#include "debug/assert.h"

namespace LAPIC {
namespace Timer {

/*! \brief Timer Delivery Status */
enum DeliveryStatus {
	IDLE         = 0,
	SEND_PENDING = 1
};

/*! \brief Timer Mode */
enum TimerMode {
	ONE_SHOT = 0,
	PERIODIC = 1,
	DEADLINE = 2
	// reserved
};

/*! \brief Timer Mask */
enum Mask {
	NOT_MASKED = 0,
	MASKED     = 1
};

static const Register INVALID_DIV = 0xff;

/*! \brief LAPIC-Timer Control Register
 *
 * \see [ISDMv3 10.5.1 Local Vector Table](intel_manual_vol3.pdf#page=375)
 */
union ControlRegister {
	struct {
		uint32_t vector                :  8;  ///< Vector
		uint32_t                       :  4;
		DeliveryStatus delivery_status :  1;  ///< Delivery Status
		uint32_t                       :  3;
		Mask masked                    :  1;  ///< Interrupt Mask (if set, interrupt will not trigger)
		TimerMode timer_mode           :  2;  ///< Timer Mode
		uint32_t                       : 13;
	};
	Register value;
} __attribute__((packed));

/*! \brief LAPIC timer divider table
 *
 * \see [ISDMv3 10.5.4 APIC Timer](intel_manual_vol3.pdf#page=378)
 */
static const Register div_masks[] = {
	0xb,  ///< divides by   1
	0x0,  ///< divides by   2
	0x1,  ///< divides by   4
	0x2,  ///< divides by   8
	0x3,  ///< divides by  16
	0x8,  ///< divides by  32
	0x9,  ///< divides by  64
	0xa   ///< divides by 128
};

/*! \brief Calculate the bit mask for the LAPIC-timer divider.
 *  \param div Divider, must be power of two: 1, 2, 4, 8, 16, 32, 64, 128
 *  \return Bit mask for LAPIC::Timer::set() or `0xff` if `div` is invalid.
 */
Register getClockDiv(uint8_t div) {
	// div is zero or not a power of two?
	if (div == 0 || (div & (div - 1)) != 0) {
		return INVALID_DIV;
	}

	int trail = __builtin_ctz(div);  // count trailing 0-bits
	if (trail > 7) {
		return INVALID_DIV;
	}

	return div_masks[trail];
}

uint32_t ticks(void) {
	uint32_t start = 1000;
	set(start, 1, Core::Interrupt::TIMER, false, true);

	assert(PIT::set(1000));
	assert(PIT::waitForTimeout());
	PIT::disable();

	uint32_t end = read(TIMER_CURRENT_COUNTER);
	return start - end;
}

void set(uint32_t counter, uint8_t divide, uint8_t vector, bool periodic, bool masked) {
	ControlRegister control_register;
	control_register.value = read(TIMER_CONTROL);
	control_register.timer_mode = periodic ? PERIODIC : ONE_SHOT;
	control_register.masked = masked ? MASKED : NOT_MASKED;
	control_register.vector = vector;
	write(TIMER_CONTROL, control_register.value);

	Register clockDiv = getClockDiv(divide);
	assert(clockDiv != INVALID_DIV);
	write(TIMER_DIVIDE_CONFIGURATION, clockDiv);

	Register initial = counter;
	write(TIMER_INITIAL_COUNTER, initial);
}

}  // namespace Timer
}  // namespace LAPIC

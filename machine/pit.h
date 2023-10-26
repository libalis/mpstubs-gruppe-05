/*! \file
 *  \brief The old/historical \ref PIT "Programmable Interval Timer (PIT)"
 */

#pragma once

#include "types.h"

/*! \brief Abstraction of the historical Programmable Interval Timer (PIT).
 *
 * Historically, PCs had a Timer component of type 8253 or 8254, modern systems come with a compatible chip.
 * Each of these chips provides three 16-bit wide counters ("channel"), each running at a frequency of 1.19318 MHz.
 * The timer's counting speed is thereby independent from the CPU frequency.
 *
 * Traditionally, the first counter (channel 0) was used for triggering interrupts, the second one (channel 1) controlled
 * the memory refresh, and the third counter (channel 2) was assigned to the PC speaker.
 *
 * As the PIT's frequency is fixed to a constant value of 1.19318 MHz, the PIT can be used for calibration.
 * For this purpose, we use channel 2 only.
 *
 * \note Interrupts should be disabled while configuring the timer.
 */
namespace PIT {

/*! \brief Start timer
 *
 *  Sets the channel 2 timer to the provided value and starts counting.
 *
 *  \note The maximum waiting time is approx. 55 000 us due to the timers being limited to 16 bit.
 *  \param us Waiting time in us
 *  \return `true` if the counter is running; `false` if the waiting time exceeds the limits.
 */
bool set(uint16_t us);

/*! \brief Reads the current timer value
 *  \return Current timer value
 */
uint16_t get(void);

/*! \brief Check if the timer is running
 *  \return `true` if running, `false` otherwise
 */
bool isActive(void);

/*! \brief (Active) waiting for timeout
 *  \return `true` when timeout was successfully hit, `false` if the timer was not active prior to calling.
 */
bool waitForTimeout(void);

/*! \brief Set the timer and wait for timeout
 *  \note The maximum waiting time is approx. 55 000 us due to the timers being limited to 16 bit.
 *  \param us Waiting time in us
 *  \return `true` when waiting successfully terminated; `false` on error (e.g., waiting time exceeds its limits)
 */
bool delay(uint16_t us);

/*! \brief Play a given frequency on the PC speaker.
 *
 *  As the PC speaker is connected to PIT channel 2, the PIT can be used to play an acoustic signal.
 *  Playing sounds occupies the PIT, so it cannot be used for other purposes while playback.
 *
 *  \note Not every PC has an activated PC speaker
 *  \note Qemu & KVM have to be launched with `-audiodev`
 *        If you still cannot hear anything, try to set `QEMU_AUDIO_DRV` to `alsa`
 *        (by launching \StuBS with `QEMU_AUDIO_DRV=alsa make kvm`)
 * \param freq Frequency (in Hz) of the sound to be played, or 0 to deactivate playback.
 */
void pcspeaker(uint32_t freq);

/*! \brief Deactivate the timer
 */
void disable(void);

}  // namespace PIT

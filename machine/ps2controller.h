/*! \file
 *  \brief \ref PS2Controller "PS/2 Controller" (Intel 8042, also known as Keyboard Controller)
 */

#pragma once

#include "types.h"
#include "object/key.h"

/*! \brief PS/2 Controller
 * \ingroup io
 *
 * Initializes the PS/2 devices (Keyboard and optional Mouse), and
 * determines both the scan code and ASCII character of a pressed key from the
 * transmitted make and break codes using the \ref KeyDecoder.
 *
 * \note This controller is also known as Intel 8042 (nowadays integrated in
 *       the mainboard) or *Keyboard Controller*.
 *       But to avoid confusion with the actual Keyboard and since we use the
 *       PS/2-compatible mode to support the Mouse as well, the name
 *       PS/2 Controller was chosen for the sake of simplicity.
 *
 * \note Since modern PCs sometimes don't have an PS/2 connector, USB keyboards
 *       and mice are emulated as PS/2 device with USB Legacy Support.
 */
namespace PS2Controller {
/*! \brief Initialization of connected devices
 *
 * All status LEDs of the keyboard are switched off and the repetition rate is
 * set to maximum speed.
 */
void init();

/*! \brief Retrieve the keyboard event
 *
 * Retrieves make and brake events from the keyboard.
 * If a valid (non special) key was pressed, the scan code is determined
 * using \ref KeyDecoder::decode into a \ref Key object.
 * Events on special keys like \key{Shift}, \key{Alt}, \key{CapsLock} etc. are stored
 * (in \ref KeyDecoder) and applied on subsequent keystrokes,
 * while no valid key is retrieved.
 *
 * Mouse events are ignored.
 *
 *  \todo Implement Method
 * \param pressed Reference to an object which will contain the pressed \ref Key on success
 * \return `true` if a valid key was decoded
 */
bool fetch(Key &pressed);

/*! \brief Delay before the keyboard starts repeating sending a pressed key
 */
enum Delay {
	DELAY_250MS  = 0,  ///< Delay of 0.25s
	DELAY_500MS  = 1,  ///< Delay of 0.5s
	DELAY_750MS  = 2,  ///< Delay of 0.75s
	DELAY_1000MS = 3   ///< Delay of 1s
};

/*! \brief Repeat Rate of Characters
 *
 * \see \ref ps2keyboard
 */
enum Speed {
	SPEED_30_0CPS  = 0x00,  ///< 30 characters per second
	SPEED_26_7CPS  = 0x01,  ///< 26.7 characters per second
	SPEED_24_0CPS  = 0x02,  ///< 24 characters per second
	SPEED_21_8CPS  = 0x03,  ///< 12.8 characters per second
	SPEED_20_7CPS  = 0x04,  ///< 20.7 characters per second
	SPEED_18_5CPS  = 0x05,  ///< 18.5 characters per second
	SPEED_17_1CPS  = 0x06,  ///< 17.1 characters per second
	SPEED_16_0CPS  = 0x07,  ///< 16 characters per second
	SPEED_15_0CPS  = 0x08,  ///< 15 characters per second
	SPEED_13_3CPS  = 0x09,  ///< 13.3 characters per second
	SPEED_12_0CPS  = 0x0a,  ///< 12 characters per second
	SPEED_10_9CPS  = 0x0b,  ///< 10.9 characters per second
	SPEED_10_0CPS  = 0x0c,  ///< 10 characters per second
	SPEED_09_2CPS  = 0x0d,  ///< 9.2 characters per second
	SPEED_08_6CPS  = 0x0e,  ///< 8.6 characters per second
	SPEED_08_0CPS  = 0x0f,  ///< 8 characters per second
	SPEED_07_5CPS  = 0x10,  ///< 7.5 characters per second
	SPEED_06_7CPS  = 0x11,  ///< 6.7 characters per second
	SPEED_06_0CPS  = 0x12,  ///< 6 characters per second
	SPEED_05_5CPS  = 0x13,  ///< 5.5 characters per second
	SPEED_05_0CPS  = 0x14,  ///< 5 characters per second
	SPEED_04_6CPS  = 0x15,  ///< 4.6 characters per second
	SPEED_04_3CPS  = 0x16,  ///< 4.3 characters per second
	SPEED_04_0CPS  = 0x17,  ///< 4 characters per second
	SPEED_03_7CPS  = 0x18,  ///< 3.7 characters per second
	SPEED_03_3CPS  = 0x19,  ///< 3.3 characters per second
	SPEED_03_0CPS  = 0x1a,  ///< 3 characters per second
	SPEED_02_7CPS  = 0x1b,  ///< 2.7 characters per second
	SPEED_02_5CPS  = 0x1c,  ///< 2.5 characters per second
	SPEED_02_3CPS  = 0x1d,  ///< 2.3 characters per second
	SPEED_02_1CPS  = 0x1e,  ///< 2.1 characters per second
	SPEED_02_0CPS  = 0x1f,  ///< 2 characters per second
};

/*! \brief Configure the repeat rate of the keyboard
 *
 * \param delay configures how long a key must be pressed before the repetition begins.
 * \param speed determines how fast the key codes should follow each other.
 *              Valid values are between `0` (30 characters per second) and
 *              `31` (2 characters per second).
 *
 *  \todo Implement method
 */
void setRepeatRate(Speed speed, Delay delay);

/*! \brief Keyboard LEDs
 */
enum LED {
	LED_SCROLL_LOCK = 1 << 0,     ///< Scroll Lock
	LED_NUM_LOCK    = 1 << 1,     ///< Num Lock
	LED_CAPS_LOCK   = 1 << 2,     ///< Caps Lock
};

/*! \brief Enable or disable a keyboard LED
 *
 *  \param led LED to enable or disable
 *  \param on `true` will enable the specified LED, `false` disable
 *
 *  \todo Implement method
 */
void setLed(enum LED led, bool on);

}  // namespace PS2Controller

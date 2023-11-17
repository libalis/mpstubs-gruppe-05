#include "machine/ps2controller.h"
#include "compiler/fix.h"
#include "debug/output.h"
#include "machine/ioport.h"
#include "machine/keydecoder.h"
#include "sync/ticketlock.h"

namespace PS2Controller {

// I/O Ports of the PS2 Controller
static const IOPort ctrl_port(0x64);  ///< Access status- (read) and command (write) register
static const IOPort data_port(0x60);  ///< Access PS/2 device [keyboard] output- (read) and input (write) buffer
/* The buffers are used to communicate with the controller or the connected
 * PS/2 devices alike:
 *  - For the output buffer, the controller decides to which PS/2 device the
 *    data gets forwarded to -- by default it is the primary PS/2 device (keyboard).
 *  - The source device from which the data was gathered can be determined using
 *    the status flag (\ref IS_MOUSE).
 *
 * Please also note, that the naming of the buffer may be a bit contra-intuitive
 * since it is the perspective of the PS/2 controller due to historical reasons.
 */

// Key decoder (stores the state of the modifier keys)
static KeyDecoder key_decoder;

// To store the current state of the Keyboard LEDs
static uint8_t MAYBE_UNUSED leds = 0;

/*! \brief Flags in the PS/2 controller status register
 */
enum Status {
	HAS_OUTPUT    = 1 << 0,  ///< Output buffer non-empty?
	INPUT_PENDING = 1 << 1,  ///< Is input buffer full?
	SYSTEM_FLAG   = 1 << 2,  ///< set on soft reset, cleared on power up
	IS_COMMAND    = 1 << 3,  ///< Is command Byte? (otherwise data)
	IS_MOUSE      = 1 << 5,  ///< Mouse output has data
	TIMEOUT_ERROR = 1 << 6,  ///< Timeout error
	PARITY_ERROR  = 1 << 7   ///< Parity error
};

/*! \brief Commands to be send to the Keyboard
 */
enum KeyboardCommand : uint8_t {
	KEYBOARD_SET_LED     = 0xed,  ///< Set the LED (according to the following parameter byte)
	KEYBOARD_SEND_ECHO   = 0xee,  ///< Send an echo packet
	KEYBOARD_SET_SPEED   = 0xf3,  ///< Set the repeat rate (according to the following parameter byte)
	KEYBOARD_ENABLE      = 0xf4,  ///< Enable Keyboard
	KEYBOARD_DISABLE     = 0xf5,  ///< Disable Keyboard
	KEYBOARD_SET_DEFAULT = 0xf6,  ///< Load defaults
};

/*! \brief Replies
 */
enum Reply {
	ACK    = 0xfa,   ///< Acknowledgement
	RESEND = 0xfe,   ///< Request to resend (not required to implement)
	ECHO   = 0xee    ///< Echo answer
};

/*! \brief Commands for the PS/2 Controller
 *
 * These commands are processed by the controller and *not* send to keyboard/mouse.
 * They have to be written into the command register.
 */
enum ControllerCommand {
	CONTROLLER_GET_COMMAND_BYTE = 0x20,  ///< Read Command Byte of PS/2 Controller
	CONTROLLER_SET_COMMAND_BYTE = 0x60,  ///< Write Command Byte of PS/2 Controller
	CONTROLLER_MOUSE_DISABLE    = 0xa7,  ///< Disable mouse interface
	CONTROLLER_MOUSE_ENABLE     = 0xa8,  ///< Enable mouse interface
	CONTROLLER_KEYBOARD_DISABLE = 0xad,  ///< Disable keyboard interface
	CONTROLLER_KEYBOARD_ENABLE  = 0xae,  ///< Enable keyboard interface
	CONTROLLER_SEND_TO_MOUSE    = 0xd4,  ///< Send parameter to mouse device
};

/*! \brief Send a command or data to a connected PS/2 device
 *
 * The value must only be written into the input buffer after the previously
 * written values have been fetched (\ref INPUT_PENDING in the status register).
 *
 *  \todo Implement method
 *
 *  \param value data to be sent
 */
static void MAYBE_UNUSED sendData(uint8_t value) {
	// TODO: You have to implement this method
	while ((ctrl_port.inb() & INPUT_PENDING) == INPUT_PENDING) {}
	data_port.outb(value);
}

void init() {
	// Switch all LEDs off (on many PCs NumLock is turned on after power up)
	setLed(LED_CAPS_LOCK, false);
	setLed(LED_SCROLL_LOCK, false);
	setLed(LED_NUM_LOCK, false);

	// Set to maximum speed & minimum delay
	setRepeatRate(SPEED_30_0CPS, DELAY_250MS);
}

bool fetch(Key &pressed) {
	// TODO: You have to implement this method
	uint8_t c;
	fetchLock.lock();
	while (((c = ctrl_port.inb()) & (HAS_OUTPUT | IS_MOUSE)) != HAS_OUTPUT)
		if ((c & (HAS_OUTPUT | IS_MOUSE)) == (HAS_OUTPUT | IS_MOUSE)) data_port.inb();
	unsigned char code = data_port.inb();
	pressed = key_decoder.decode(code);
	fetchLock.unlock();
	return pressed.valid();
}

void setRepeatRate(Speed speed, Delay delay) {
	// TODO: You have to implement this method. Use sendData()
	sendData(KEYBOARD_SET_SPEED);
	uint8_t parameter_byte = delay << 5;
	parameter_byte |= speed;
	sendData(parameter_byte);
}

void setLed(enum LED led, bool on) {
	// TODO: You have to implement this method. Use sendData()
	sendData(KEYBOARD_SET_LED);
	if (on) leds |= led;
	else
		leds &= ~led;
	sendData(leds);
}

void drainBuffer() {
	while ((ctrl_port.inb() & HAS_OUTPUT) == HAS_OUTPUT) data_port.inb();
}

}  // namespace PS2Controller

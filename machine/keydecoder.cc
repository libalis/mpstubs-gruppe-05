#include "machine/keydecoder.h"
#include "machine/ps2controller.h"

// Constants used for key decoding
const unsigned char BREAK_BIT = 0x80;
const unsigned char PREFIX_1  = 0xe0;
const unsigned char PREFIX_2  = 0xe1;

Key KeyDecoder::decode(unsigned char code) {
	Key key = modifier;

	// All keys that are introduced by the MF II keyboard (compared to the older AT keyboard)
	// always send a prefix value as first byte.
	if (code == PREFIX_1 || code == PREFIX_2) {
		prefix = code;
	} else {
		// Releasing a key is, for us, only important for the modifier keys such as SHIFT, CTRL and ALT,
		// For other, non-modifier keys, we ignore the break code.
		bool pressed = (code & BREAK_BIT) == 0;

		// A key's break code is identical to its make code with an additionally set BREAK_BIT
		Key::Scancode scancode = static_cast<Key::Scancode>(code & (~BREAK_BIT));

		// We ignore "new" special keys, such as the Windows key
		if (scancode < Key::Scancode::KEYS) {
			// save state
			status[scancode] = pressed;

			// Take a closer look at modifier make and break events
			bool isModifier = true;
			switch (scancode) {
				// both shifts are handled equally
				case Key::Scancode::KEY_LEFT_SHIFT:
				case Key::Scancode::KEY_RIGHT_SHIFT:
					modifier.shift = pressed;
					break;

				case Key::Scancode::KEY_LEFT_ALT:
					if (prefix == PREFIX_1) {
						modifier.alt_right = pressed;
					} else {
						modifier.alt_left = pressed;
					}
					break;

				case Key::Scancode::KEY_LEFT_CTRL:
					if (prefix == PREFIX_1) {
						modifier.ctrl_right = pressed;
					} else {
						modifier.ctrl_left = pressed;
					}
					break;

				default:
					isModifier = false;
			}

			// For keys other than modifiers, we only care about the make code
			if (pressed && !isModifier) {
				switch (scancode) {
					case Key::Scancode::KEY_CAPS_LOCK:
						modifier.caps_lock ^= 1;
						setLed(PS2Controller::LED_CAPS_LOCK, modifier.caps_lock);
						break;

					case Key::Scancode::KEY_SCROLL_LOCK:
						modifier.scroll_lock ^= 1;
						setLed(PS2Controller::LED_SCROLL_LOCK, modifier.scroll_lock);
						break;

					case Key::Scancode::KEY_NUM_LOCK:  // Can be both NumLock and pause
						// On old keyboards, the pause functionality was only accessible by
						// pressing Ctrl+NumLock. Modern MF-II keyboards therefore send exactly
						// this code combination when the pause key was pressed.
						// Normally, the pause key does not provide an ASCII code, but we check
						// that anyway. In either case, we're now done decoding.
						if (modifier.ctrl_left) {  // pause key
							key.scancode = scancode;
						} else {  // NumLock
							modifier.num_lock ^= 1;
							setLed(PS2Controller::LED_NUM_LOCK, modifier.num_lock);
						}
						break;

					// Special case scan code 53: This code is used by both the minus key on the main
					// keyboard and the division key on the number block.
					// When the division key was pressed, we adjust the scancode accordingly.
					case Key::Scancode::KEY_SLASH:
						if (prefix == PREFIX_1) {
							key.scancode = Key::Scancode::KEY_DIV;
							key.shift = true;
						} else {
							key.scancode = scancode;
						}
						break;

					default:
						key.scancode = scancode;

						// When NumLock is enabled and a key on the keypad was pressed, we
						// want return the ASCII and scan codes of the corresponding numerical
						// key instead of the arrow keys.
						// The keys on the cursor block (prefix == PREFIX_1), however, should
						// remain usable. Therefore, as a little hack, we deactivate the NumLock
						// for these keys.
						if (modifier.num_lock && prefix == PREFIX_1) {
							key.num_lock = false;
						}
				}
			}
		}

		// The prefix is only valid for the immediately following code, which was just handled.
		prefix = 0;
	}

	return key;
}

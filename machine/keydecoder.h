/*! \file
 *  \brief \ref KeyDecoder decodes a keystroke to the corresponding \ref Key object
 */

#pragma once

#include "object/key.h"

/*! \brief Decoder for \ref ps2keyboardset1 "keyboard codes" received from the \ref PS2Controller
 *  \ingroup io
 *
 *  Extracts the make and break codes, modifier and scan codes from the pressed key.
 */
class KeyDecoder {
	unsigned char prefix;  ///< Prefix byte for keys
	Key modifier;          ///< activated modifier keys (e.g., caps lock)

 public:
	/*! \brief Current state (pressed or released) of all keys.
	 */
	bool status[Key::Scancode::KEYS];

	/*! \brief Default constructor
	 */
	KeyDecoder() {}

	/*! \brief Interprets the  \ref ps2keyboardset1 "make and break codes" received from the
	 *         keyboard and derives the corresponding scan code and further information about
	 *         other pressed keys, such as \key{shift} and \key{ctrl}.
	 *
	 * \param code Byte from Keyboard to decode
	 * \return Pressed key (\ref Key::valid returns `false` if the key is not yet complete)
	 */
	Key decode(unsigned char code);
};

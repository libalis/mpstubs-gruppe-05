/*! \file
 *  \brief \ref Key, an abstraction for handling pressed keys and their modifiers
 */

#pragma once

#include "types.h"

/*! \brief Class that abstracts a key, made up of the scan code and the modifier bits.
 */
struct Key {
	/*! \brief The keys' scan codes (code 1)
	 */
	enum Scancode : uint8_t {
		// Invalid scan code
		KEY_INVALID = 0,

		// "real" valid scan codes
		KEY_ESCAPE,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_0,
		KEY_DASH,
		KEY_EQUAL,
		KEY_BACKSPACE,
		KEY_TAB,
		KEY_Q,
		KEY_W,
		KEY_E,
		KEY_R,
		KEY_T,
		KEY_Y,
		KEY_U,
		KEY_I,
		KEY_O,
		KEY_P,
		KEY_OPEN_BRACKET,
		KEY_CLOSE_BRACKET,
		KEY_ENTER,
		KEY_LEFT_CTRL,
		KEY_A,
		KEY_S,
		KEY_D,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_SEMICOLON,
		KEY_APOSTROPH,
		KEY_GRAVE_ACCENT,
		KEY_LEFT_SHIFT,
		KEY_BACKSLASH,
		KEY_Z,
		KEY_X,
		KEY_C,
		KEY_V,
		KEY_B,
		KEY_N,
		KEY_M,
		KEY_COMMA,
		KEY_PERIOD,
		KEY_SLASH,
		KEY_RIGHT_SHIFT,
		KEY_KP_STAR,
		KEY_LEFT_ALT,
		KEY_SPACEBAR,
		KEY_CAPS_LOCK,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_NUM_LOCK,
		KEY_SCROLL_LOCK,
		KEY_KP_7,
		KEY_KP_8,
		KEY_KP_9,
		KEY_KP_DASH,
		KEY_KP_4,
		KEY_KP_5,
		KEY_KP_6,
		KEY_KP_PLUS,
		KEY_KP_1,
		KEY_KP_2,
		KEY_KP_3,
		KEY_KP_0,
		KEY_KP_PERIOD,
		KEY_SYSREQ,
		KEY_EUROPE_2,
		KEY_F11,
		KEY_F12,
		KEY_KP_EQUAL,

		// Number of keys (excluding aliases below)
		KEYS,

		// aliases
		KEY_DIV   = KEY_7,
		KEY_DEL   = KEY_KP_PERIOD,
		KEY_UP    = KEY_KP_8,
		KEY_DOWN  = KEY_KP_2,
		KEY_LEFT  = KEY_KP_4,
		KEY_RIGHT = KEY_KP_6,
	};

	Scancode scancode;

	// bit masks for the modifier keys
	bool shift       : 1,
	     alt_left    : 1,
	     alt_right   : 1,
	     ctrl_left   : 1,
	     ctrl_right  : 1,
	     caps_lock   : 1,
	     num_lock    : 1,
	     scroll_lock : 1;

	/*! \brief Default constructor: Instantiates an invalid key by setting ASCII, scan code, and modifier bits to 0
	 */
	Key() : scancode(KEY_INVALID), shift(false), alt_left(false), alt_right(false),
	        ctrl_left(false), ctrl_right(false),
	        caps_lock(false), num_lock(false), scroll_lock(false) {}

	/*! \brief Invalid keys have a scancode = 0
	 *  \return Checks whether a key is valid.
	 */
	bool valid() const {
		return scancode != KEY_INVALID && scancode < KEYS;
	}

	/*! \brief Marks the key as invalid by setting the scan code to 0.
	 *
	 */
	void invalidate() {
		scancode = KEY_INVALID;
	}

	/*! \brief Get the key's ASCII value
	 *  \return the key's ASCII value
	 */
	unsigned char ascii() const;

	/*! \brief Indicates whether the ALT modifier is set
	 *  \return `true` if ALT key was pressed during key press
	 */
	bool alt() const  {
		return alt_left || alt_right;
	}

	/*! \brief Indicates whether the CTRL modifier is set
	 *  \return `true` if CTRL key was pressed during key press
	 */
	bool ctrl() const {
		return ctrl_left || ctrl_right;
	}

	/*! \brief Conversion to char (ASCII code)
	 *
	 */
	operator char() const {  //NOLINT since we want implicit conversions
		return static_cast<char>(ascii());
	}
};

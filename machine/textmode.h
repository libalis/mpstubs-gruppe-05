/*! \file
 *  \brief \ref TextMode provides a basic interface to display a character in VGA-compatible text mode
 */

#pragma once

#include "types.h"

/*! \brief Basic operations in the VGA-compatible text mode
 *  \ingroup io
 *
 * This class provides an interface to access the screen in text mode, with
 * access directly on the hardware level, i.e. the video memory and the
 * I/O ports of the graphics card.
 */
class TextMode {
 public:
	static const unsigned ROWS    = 25;  ///< Visible rows in text mode
	static const unsigned COLUMNS = 80;  ///< Visible columns in text mode

	/*! \brief CGA color palette
	 *
	 * Colors for the attribute byte.
	 * All 16 colors can be used for the foreground while the background colors
	 * are limited to the first eight (from`BLACK` to `LIGHT_GREY`)
	 */
	enum Color {
		BLACK,          ///< Black (fore- and background)
		BLUE,           ///< Blue (fore- and background)
		GREEN,          ///< Green (fore- and background)
		CYAN,           ///< Cyan (fore- and background)
		RED,            ///< Red (fore- and background)
		MAGENTA,        ///< Magenta (fore- and background)
		BROWN,          ///< Brown (fore- and background)
		LIGHT_GREY,     ///< Light grey (fore- and background)
		DARK_GREY,      ///< Dark grey (foreground only)
		LIGHT_BLUE,     ///< Light blue (foreground only)
		LIGHT_GREEN,    ///< Light green (foreground only)
		LIGHT_CYAN,     ///< Light cyan (foreground only)
		LIGHT_RED,      ///< Light red (foreground only)
		LIGHT_MAGENTA,  ///< Light magenta (foreground only)
		YELLOW,         ///< Yellow (foreground only)
		WHITE           ///< White (foreground only)
	};

	/*! \brief Structure of a character attribute
	 * consists of 4 bit fore- and 3 bit background color, and a single blink bit.
	 *
	 * [Bit fields](https://en.cppreference.com/w/cpp/language/bit_field) can
	 * notably simplify the access and code readability.
	 *
	 * \note [Type punning](https://en.wikipedia.org/wiki/Type_punning#Use_of_union)
	 *       is indeed undefined behavior in C++. However, *gcc* explicitly allows this construct as a
	 *       [language extension](https://gcc.gnu.org/bugs/#nonbugs).
	 *       Some compilers ([other than gcc](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Type%2Dpunning)
	 *       might allow this feature only by disabling strict aliasing (`-fno-strict-aliasing`).
	 *       In \StuBS we use this feature extensively due to the improved code readability.
	 */
	union Attribute {
		struct {
			uint8_t foreground : 4;  ///< `.... XXXX` Foreground color
			uint8_t background : 3;  ///< `.XXX ....` Background color
			uint8_t blink      : 1;  ///< `X... ....` Blink
		} __attribute__((packed));
		uint8_t value;  ///< combined value

		/*! \brief Attribute constructor (with default values)
		 *
		 *  \todo Complete constructor
		 *
		 *  \param foreground Foreground color (Default: \ref LIGHT_GREY)
		 *  \param background Background color (Default: \ref BLACK)
		 *  \param blink Blink if `true` (default: no blinking)
		 */
		explicit Attribute(Color foreground = LIGHT_GREY, Color background = BLACK, bool blink = false)
			 {  //NOLINT
				(void) foreground;
				(void) background;
				(void) blink;
			}

	} __attribute__((packed));  // prevent padding by the compiler

	/*! \brief Set the keyboard hardware cursor to absolute screen position
	 *
	 *  \todo Implement the method using \ref IOPort
	 *
	 *  \param abs_x absolute column of the keyboard hardware cursor
	 *  \param abs_y absolute row of the keyboard hardware cursor
	 */
	static void setCursor(unsigned abs_x, unsigned abs_y);

	/*! \brief Retrieve the keyboard hardware cursor position on screen
	 *
	 *  \todo Implement the method using the \ref IOPort
	 *
	 *  \param abs_x absolute column of the keyboard hardware cursor
	 *  \param abs_y absolute row of the keyboard hardware cursor
	 */
	static void getCursor(unsigned& abs_x, unsigned& abs_y);

	/*! \brief Basic output of a character at a specific position on the screen.
	 *
	 * This method outputs the given character at the absolute screen position
	 * (`x`, `y`) with the specified color attribute.
	 *
	 * The position (`0`,`0`) indicates the upper left corner of the screen.
	 * The attribute defines characteristics such as background color,
	 * foreground color and blinking.
	 *
	 * \param abs_x Column (`abs_x` < \ref COLUMNS) in which the character should be displayed
	 * \param abs_y Row (`abs_y` < \ref ROWS) in which the character should be displayed
	 * \param character Character to be displayed
	 * \param attrib Attribute with color settings
	 * \todo Implement the method
	 */
	static void show(unsigned abs_x, unsigned abs_y, char character, Attribute attrib = Attribute());

};

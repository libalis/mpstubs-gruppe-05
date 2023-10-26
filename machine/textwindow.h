/*! \file
 *  \brief \ref TextWindow provides virtual output windows in text mode
 */

#pragma once

#include "types.h"
#include "machine/textmode.h"

/*! \brief Virtual windows in text mode
 *  \ingroup io
 *
 * Outputs text on a part of the screen in \ref TextMode "text mode",
 * a window defined in its position and size (with its own cursor).
 *
 * This allows to separate the output of the application from the debug output
 * on the screen without having to synchronize.
 */
class TextWindow : public TextMode {
	// Prevent copies and assignments
	TextWindow(const TextWindow&)            = delete;
	TextWindow& operator=(const TextWindow&) = delete;

 public:
	/*! \brief Constructor of a text window
	 *
	 * Creates a virtual, rectangular text window on the screen.
	 * The coordinates to construct the window are absolute positions in the
	 * \ref TextMode screen.
	 *
	 * \note Overlapping windows are neither supported nor prevented -- better
	 *       just try to avoid construction windows with overlapping coordinates!
	 *
	 * \warning Don't use the hardware cursor in more than one window!
	 *
	 * \param from_col   Text Window starts in column `from_col`,
	 *                   the first (leftmost) possible column is `0`
	 * \param to_col     Text Window extends to the right to column `to_col` (exclusive).
	 *                   This column has to be strictly greater than `from_col`,
	 *                   the maximum allowed value is \ref TextMode::COLUMNS (rightmost)
	 * \param from_row   Text Window starts in row `from_row`,
	 *                   the first possible (uppermost) row is `0`
	 * \param to_row     Text Window extends down to row `to_row` (exclusive).
	 *                   This row has to be strictly greater than `from_row`,
	 *                   the maximum allowed value is \ref TextMode::ROWS (bottom-most)
	 * \param use_cursor Specifies whether the hardware cursor (`true`) or a
	 *                   software cursor/variable (`false`) should be used to
	 *                   store the current position
	 *
	 *  \todo Implement constructor
	 */
	TextWindow(unsigned from_col, unsigned to_col, unsigned from_row, unsigned to_row, bool use_cursor = false);

	/*! \brief Set the cursor position in the window
	 *
	 * Depending on the constructor parameter `use_cursor` either the
	 * hardware cursor (and only the hardware cursor!) is used or the position
	 * is stored internally in the object.
	 *
	 * The coordinates are relative to the upper left starting position of
	 * the window.
	 *
	 *  \param rel_x Column in window
	 *  \param rel_y Row in window
	 *  \todo Implement method, use \ref TextMode::setCursor() for the hardware cursor
	 */
	void setPos(unsigned rel_x, unsigned rel_y);

	/*! \brief Get the current cursor position in the window
	 *
	 * Depending on the constructor parameter `use_cursor` either the
	 * hardware cursor (and only the hardware cursor!) is used or the position
	 * is retrieved from the internally stored object.
	 *
	 *  \param rel_x Column in window
	 *  \param rel_y Row in window
	 *  \todo Implement Method, use \ref TextMode::getCursor() for the hardware cursor
	 */
	void getPos(unsigned& rel_x, unsigned& rel_y) const;

	/*! \brief Display multiple characters in the window
	 *
	 * Output a character string, starting at the current cursor position.
	 * Since the string does not need to contain a `\0` termination (unlike the
	 * common C string), a length parameter is required to specify the number
	 * of characters in the string.
	 * When the output is complete, the cursor is positioned after the last
	 * printed character.
	 * The same attributes (colors) are used for the entire text.
	 *
	 * If there is not enough space left at the end of the line,
	 * the output continues on the following line.
	 * As soon as the last window line is filled, the entire window area is
	 * moved up one line: The first line disappears, the bottom line is cleared.
	 *
	 * A line break also occurs whenever the character `\n` appears in the text.
	 *
	 *  \param string Text to be printed
	 *  \param length Length of text
	 *  \param attrib Attribute for text
	 *  \todo Implement Method
	 */
	void print(const char* string, size_t length, Attribute attrib = TextMode::Attribute()); //NOLINT

	/*! \brief Delete all contents in the window and reset the cursor.
	 *
	 *  \param character Fill character
	 *  \param attrib Attribute for fill character
	 *  \todo Implement Method
	 */
	void reset(char character = ' ', Attribute attrib = TextMode::Attribute());
};

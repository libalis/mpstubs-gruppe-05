/*! \file
 *  \brief \ref GraphicsStream, a \ref Graphics "graphical" \ref OutputStream "output stream"
 *  inspired by (and compatible to) \ref TextStream
 */

#pragma once

#include "object/outputstream.h"
#include "graphics/primitives.h"
#include "device/graphics.h"

/*! \brief Output text (form different data type sources) on screen in \ref Graphics "graphic mode" (similar to \ref TextStream)
 * \ingroup gfx
 * \ingroup io
 *
 * Enables output of different data types using a monospaced font on a
 * predefined area of the screen with activated graphics mode.
 */
class GraphicsStream : public OutputStream {
	// Prevent copies and assignments
	GraphicsStream(const GraphicsStream&)            = delete;
	GraphicsStream& operator=(const GraphicsStream&) = delete;

	struct Cell {
		char character;
		Color color;
	};

	Cell * cell;
	unsigned offset;

	Graphics &graphics;

	unsigned x, y;       ///< Cursor position

 protected:
	/*! \brief Output the buffer contents of the base class \ref Stringbuffer
	 *
	 * The method is automatically called when the buffer is full,
	 * but can also be called explicitly to force output of the current buffer.
	 */
	void flush();

 public:
	Font * const FONT;       ///< Default font
	const Point START;       ///< Upper left corner of the window
	const unsigned ROWS;     ///< Number of rows in the window
	const unsigned COLUMNS;  ///< Number of columns in the window

	/*! \brief CGA color palette
	 */
	static const Color BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GREY,
	DARK_GREY, LIGHT_BLUE, LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED,
	LIGHT_MAGENTA, YELLOW, WHITE;

	/*! \brief Constructor
	 *
	 * Creates a window (= area on the screen) for text output.
	 * Within the window text uses a virtual (invisible) cursor to offer a
	 * very similar behavior to \ref TextStream -- including automatic
	 * scrolling and column/row based positioning.
	 *
	 * \param graphics Graphics device for output
	 * \param start Coordinate of the upper left corner for the output window
	 * \param width Width of the output window
	 * \param height Height of the output window
	 * \param font Font used for output text (or `nullptr` for default font)
	 */
	GraphicsStream(Graphics &graphics, const Point &start, unsigned width, unsigned height, Font * font = nullptr);

	/*! \brief Set the cursor position
	 *
	 *  \param x Column in window
	 *  \param y Row in window
	 */
	void setPos(int x, int y);

	/*! \brief Read the current cursor position
	 *
	 *  \param x Column in window
	 *  \param y Row in window
	 */
	void getPos(int& x, int& y) const;

	/*! \brief Display multiple characters in the window starting at the current cursor position
	 *
	 * This method can be used to output a string, starting at the current cursor
	 * position. Since the string does not need to contain a '\0' termination
	 * (as it is usually the case in C), the parameter `length` is required to
	 * specify the number of characters in the string.
	 * When the output is complete, the cursor will be positioned after the
	 * last character printed. The entire text uniformly has the color `color`
	 *
	 * If there is not enough space left at the end of the line, the output will
	 * be continued on the following line. As soon as the last window line is
	 * filled, the entire window area will be moved up one line.
	 * The first line disappears and the last line is blank, continuing output
	 * there.
	 *
	 * A line break will also occurs wherever the character `\\n` is inserted
	 * in the text to be output.
	 *
	 *  \param str String to output
	 *  \param length length of string
	 *  \param color Foreground color of string
	 */
	void print(char* str, int length, const Color &color = LIGHT_GREY);

	/*! \brief Clear window and reset cursor
	 *
	 *  \param character Filling character
	 *  \param color Foreground color
	 */
	void reset(char character = ' ', const Color &color = LIGHT_GREY);

	/*! \brief Basic output of a (colored) character at a certain position on
	 * the screen.
	 *
	 * Outputs `character` at the absolute position (`x`, `y`) with the
	 * specified color: `x` specifies the column and `y` the row of the desired
	 * position, with 0 ≤ x < \ref COLUMNS and 0 ≤ `y` < \ref ROWS.
	 * The position (0,0) indicates the upper left corner of the window (at
	 * the coordinates \ref START).
	 *
	 *  \param x Column for output of the character
	 *  \param y Row for output of the character
	 *  \param character Character to be output
	 *  \param color Foreground color
	 */
	void show(int x, int y, char character, const Color &color = LIGHT_GREY);

	/*! \brief Draw using the \ref Graphics device
	 */
	void draw();
};

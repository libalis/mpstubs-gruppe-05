/*! \file
 *  \brief \ref TextStream outputs text onto the screen in \ref TextMode
 */

/*! \defgroup io I/O subsystem
 *  \brief The input/output subsystem
 */

#pragma once

#include "object/outputstream.h"
#include "machine/textwindow.h"

/*! \brief  Output text (form different data type sources) on screen in text mode
 *  \ingroup io
 *
 * Allows the output of different data types as strings on the \ref TextMode
 * screen of a PC.
 * To achieve this, \ref TextStream is derived from both \ref OutputStream and
 * \ref TextWindow and only implements the method \ref TextStream::flush().
 * Further formatting or special effects are implemented in \ref TextWindow.
 */
class TextStream {
	// Prevent copies and assignments
	TextStream(const TextStream&)            = delete;
	TextStream& operator=(const TextStream&) = delete;
 public:
	/// \copydoc TextWindow::TextWindow(unsigned,unsigned,unsigned,unsigned,bool)
	TextStream(unsigned from_col, unsigned to_col, unsigned from_row, unsigned to_row, bool use_cursor = false) {
		(void) from_col;
		(void) to_col;
		(void) from_row;
		(void) to_row;
		(void) use_cursor;
	}

	/*! \brief Output the buffer contents of the base class \ref Stringbuffer
	 *
	 * The method is automatically called when the buffer is full,
	 * but can also be called explicitly to force output of the current buffer.
	 *
	 *
	 *  \todo Implement method
	 */
	void flush();
};

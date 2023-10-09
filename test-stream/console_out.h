/*! \file
 *  \brief \ref ConsoleOut "Console" \ref OutputStream "output" (for the voluntary C++ exercise only)
 */

#pragma once

#include "outputstream.h"

/*! \brief Write text on console (`STDOUT`)
 *
 * This class allows writing to the console similar to `std::cout` from the standard C++ library.
 * The class is derived from \ref OutputStream.
 */
class ConsoleOut : public OutputStream {
	// Prevent copies and assignments
	ConsoleOut(const ConsoleOut&)            = delete;
	ConsoleOut& operator=(const ConsoleOut&) = delete;

 public:
	/*! \brief Constructor
	 *
	 *  \todo Implement constructor
	 */
	ConsoleOut();

	/*! \brief Output the string on the screen.
	 *
	 * The implementation should solely use `putchar()`
	 *
	 *  \todo Implement virtual method
	 */
	virtual void flush() override;  //NOLINT
};

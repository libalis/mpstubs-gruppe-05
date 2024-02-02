/*! \file
 *  \brief \ref KeyboardApplication to test the input
 */

#pragma once

#include "thread/thread.h"

/*! \brief Keyboard Application
 */
class KeyboardApplication : public Thread {
	// Prevent copies and assignments
	KeyboardApplication(const KeyboardApplication&)            = delete;
	KeyboardApplication& operator=(const KeyboardApplication&) = delete;

 public:
	/*! \brief Constructor
	 */
	KeyboardApplication() {}

	/*! \brief Contains the application code.
	 *
	 */
	void action();
};

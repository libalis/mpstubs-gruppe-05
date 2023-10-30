/*! \file
 *  \brief \ref KeyboardApplication to test the input
 */

#pragma once

/*! \brief Keyboard Application
 */
class KeyboardApplication {
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

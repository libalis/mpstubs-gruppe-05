/*! \file
 *  \brief \ref GuardedKeyboard, a \ref Guarded "guarded" interface for \ref Keyboard
 */
#pragma once

#include "device/keyboard.h"

/*! \brief syscall interface for keyboard.
 *
 */
 /*! \brief \ref Guarded interface to the \ref Keyboard used by user applications.
  *
  * Implements the system call interface for class \ref Keyboard. All methods
  * provided by this class are wrappers for the respective method from the base
  * class, which provide additional synchronization by using the class \ref Guarded.
  */
class GuardedKeyboard : public Keyboard {
	// Prevent copies and assignments
	GuardedKeyboard(const GuardedKeyboard&)            = delete;
	GuardedKeyboard& operator=(const GuardedKeyboard&) = delete;

 public:
	GuardedKeyboard() {}

	/*! \copydoc Keyboard::getKey()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref Keyboard, with the only difference that the call will be
	 *       protected by a \ref Guarded object.
	 *
	 *  \todo Implement method
	 */
	Key getKey(){
		return Key();
	}
};

/*! \file
 *  \brief \ref Guarded, an interface to secure critical sections
 */

#pragma once

/*! \brief A handy interface to protect critical sections
 *  \ingroup interrupts
 *
 * This exploits the way how the C++ compiler automatically creates constructor
 * and destructor calls in the code, especially when the scope in which an object
 * was declared is left.
 *
 * So if you \ref Guard::enter() "enter" a critical section using
 * \ref Guarded() "its constructor" and \ref Guard::leave() "leave" it again in
 * the destructor, you can easily mark critical code areas as
 * follows:
 *
 *  \code{.cpp}
 *	// non-critical section
 *	...
 *	{
 *	    Guarded section;
 *	    // critical section
 *	    ...
 *	}
 *	// non-critical section
 *	\endcode
 */
class Guarded {
	// Prevent copies and assignments
	Guarded(const Guarded&) = delete;
	Guarded& operator=(const Guarded&) = delete;

 public:
	// TODO: Implement constructor and destructor
};

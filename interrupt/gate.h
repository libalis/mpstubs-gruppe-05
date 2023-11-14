/*! \file
 *  \brief Class \ref Gate (Device interrupt handling)
 */

#pragma once

/*! \brief Class of objects that are capable of handling interrupts
 *  \ingroup interrupts
 *
 * All objects to be assigned in \ref Plugbox must be derived from this class.
 *
 * Each inheriting class must define the virtual method \ref Gate::trigger().
 */
class Gate {
	// Prevent copies and assignments
	Gate(const Gate&) = delete;
	Gate& operator=(const Gate&) = delete;

 public:
	/*! \brief Constructor
	 */
	Gate() {}

	/*! \brief Destructor
	 *
	 * Classes with virtual methods should always have a virtual destructor
	 * (which can be empty as well). In \StuBS this will calm the compiler,
	 * on other systems this will guarantee that delete will free the memory
	 * for objects of the derived classes correctly.
	 */
	virtual ~Gate() {}

	/*! \brief Device-specific interrupt handler
	 *
	 * This method is executed immediately after the interrupt occurs
	 * (asynchronously).
	 * Since it is implemented as a pure virtual method, it must be implemented
	 * by each derived classes.
	 *
	 */
	virtual void trigger() = 0;

};

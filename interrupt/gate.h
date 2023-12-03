/*! \file
 *  \brief Class \ref Gate (Device interrupt handling)
 */

#pragma once

#include "machine/core.h"

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
	Gate* next[Core::MAX];

	/*! \brief Constructor
	 */
	Gate() {
		for (uint32_t i = 0; i < Core::MAX; i++) next[i] = nullptr;
	}

	/*! \brief Destructor
	 *
	 * Classes with virtual methods should always have a virtual destructor
	 * (which can be empty as well). In \StuBS this will calm the compiler,
	 * on other systems this will guarantee that delete will free the memory
	 * for objects of the derived classes correctly.
	 */
	virtual ~Gate() {}

	virtual void epilogue() {}
	virtual bool prologue() = 0;
};

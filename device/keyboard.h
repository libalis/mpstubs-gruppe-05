/*! \file
 *  \brief The \ref Keyboard device handles keystrokes
 */

#pragma once

#include "interrupt/gate.h"
#include "debug/output.h"
#include "object/key.h"
#include "syscall/guarded_semaphore.h"

#define BUFFER_SIZE (1024)

/*! \brief Handles keystrokes.
 *  \ingroup io
 *
 * This class ensures correct initialization of the keyboard and, above all,
 * its interrupt handling.
 *
 */
class Keyboard : public Gate {
	// Prevent copies and assignments
	Keyboard(const Keyboard&)            = delete;
	Keyboard& operator=(const Keyboard&) = delete;

 private:
	Key pressed[BUFFER_SIZE];
	int counter;
	GuardedSemaphore guardedsemaphore;

 public:
	/*! \brief Constructor
	 */
	Keyboard() : counter(0), guardedsemaphore(0) {}

	/*! \brief Destructor
	 */
	~Keyboard() {
		DBG << "Keyboard::~Keyboard" << endl;
	}

	/*! \brief Initialization of the keyboard
	 *
	 * Initialization of the keyboard and activation of the specific interrupt
	 * handling: The object will register itself at the \ref Plugbox and
	 * configure the \ref IOAPIC to receive the corresponding interrupts.
	 *
	 * \note The keyboard interrupts should be configured as \ref IOAPIC::LEVEL "level triggered".
	 *       According to the standard we would have to check the corresponding entry in
	 *       \ref ACPI::MADS::Interrupt_Source_Override and use these values. Most likely this would
	 *       suggest an \ref IOAPIC::EDGE "edge-triggered mode" -- which would work as well.
	 *       However, using a \ref IOAPIC::LEVEL "level-triggered mode" is more forgiving because
	 *       it resends the interrupt request even if an interrupt was lost (e.g. the required
	 *       handling, retrieving the buffer entry, was not performed).
	 *
	 * \todo Implement Method
	 */
	void plugin();

	bool prologue() override;
	void epilogue() override;
	Key getKey();
};

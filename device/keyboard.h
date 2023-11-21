/*! \file
 *  \brief The \ref Keyboard device handles keystrokes
 */

#pragma once

#include "interrupt/gate.h"
#include "debug/output.h"

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

 public:
	/*! \brief Constructor
	 */
	Keyboard() {}

	/*! \brief Destructor
	 */
	~Keyboard() {
		DBG << "Keyboard::~Keyboard()" << endl;
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

	/*! \brief Handling of keyboard interrupts
	 *
	 * Processes interrupts triggered by the keyboard.
	 * On each keystroke it outputs the corresponding character on the screen
	 * -- only \ref Key::valid() "valid keys" are printed, for the sake of
	 * simplicity all in a separate line dedicated to the keyboard.
	 * If the key combination \key{Ctrl} + \key{Alt} + \key{Del} is pressed,
	 * a reboot is triggered.
	 *
	 *  \todo Implement Method
	 */
	void trigger() override;

};

extern Keyboard keyboard;

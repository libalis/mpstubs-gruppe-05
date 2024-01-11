/*! \file
 *  \brief Functionality required for \ref context_switch "context switching"
 */

/*! \defgroup context Context Switch
 *  \brief Low-Level functionality required for context switching
 */

#pragma once

#include "types.h"

/*! \brief Structure for saving the stack pointer when switching coroutines.
 * \ingroup context
 */
struct StackPointer {
	void *kernel;  ///< Current kernel stack pointer of the thread
} __attribute__((packed));

/*! \brief Prepares a context for its first activation.
 *
 *  \ingroup context
 *
 *  To allow the execution to start in \ref Thread::kickoff during the first activation,
 *  the stack must be initialized such that it contains all the information required.
 *  This is, most importantly, the function to be called (typically the respective
 *  implementation of \ref Thread::kickoff) and any parameters required by this function.
 *
 *  `prepareContext()` can be implemented in the high-level programming language C++
 *  (in file `context.cc`).
 *
 *  \param tos     Pointer to the top of stack (= address of first byte beyond the memory reserved for the stack)
 *  \param kickoff Pointer to the \ref Thread::kickoff function
 *  \param param1  first parameter for \ref Thread::kickoff function
 *  \return Pointer to the latest data (which will be `pop`ped first) on the prepared stack
 *
 *  \todo Implement Function (and helper functions, if required)
 */
void * prepareContext(void * tos, void (*kickoff)(void *),
                      void * param1 = nullptr);

/*! \brief Executes the context switch.
 *
 *  \ingroup context
 *
 *  For a clean context switch, the current register values must be stored to the currently active
 *  stack and the stack pointer must be stored to the `current` \ref StackPointer structure.
 *  Subsequently, these values must be restored accordingly from the `next` stack.
 *
 *  This function must be implemented in assembler in the file `context.asm`.
 *  It must be declared as `extern "C"`, as assembler functions are not C++ name mangled.
 *
 *  \param current Pointer to the structure that the current stack pointer will be stored in
 *  \param next    Pointer to the structure that the next stack pointer will be read from
 *
 *  \todo Implement Method
 */
extern "C" void context_switch(StackPointer & current, StackPointer & next);

extern "C" void prepare_parameter();

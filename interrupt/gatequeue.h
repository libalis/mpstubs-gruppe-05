/*! \file
 *  \brief \ref GateQueue "Queue" for \ref Gate "gates" (epilogues)
 */

#pragma once

#include "interrupt/gate.h"

/*! \brief Queue for \ref Gate "Gates"
 *
 * While for \OOStuBS a simple linked list is sufficient (by having a `head`
 * pointer in this calls and extending \ref Gate with a `next` pointer),
 * for multi-core systems like \MPStuBS it needs to be able to enqueue the same
 * \ref Gate item on each \ref Core (up to \ref Core::MAX) at the same time
 * (a multi-queue, requiring a `head` pointer array member in this class and
 * a `next` pointer array in \ref Gate).
 */
class GateQueue {
 public:
	/*! \brief Enqueues the provided item at the end of the queue.
	 *
	 * In \MPStuBS the item is added to the internal queue belonging to the core
	 * it is currently executed on.
	 *
	 * \param item  Queue element to be appended.
	 * \return `true` if successfully added to the queue,
	 *         `false` if it is already in the queue
	 *
	 *  \todo Implement Method
	 */
	bool enqueue(Gate *item);

	/*! \brief Removes the first element in the queue and returns it.
	 *
	 * In \MPStuBS the item is retrieved from the internal queue belonging to the
	 * core it is currently executed on.
	 *
	 *  \return Pointer to the removed item or `nullptr` if the queue was empty.
	 *
	 *  \todo Implement Method
	 */
	Gate* dequeue();
};

extern GateQueue gatequeue;

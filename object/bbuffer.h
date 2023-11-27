/*! \file
 *  \brief Contains a \ref BBuffer "bounded buffer"
 */

#pragma once

#include "types.h"

/*! \brief The class BBuffer implements a bounded buffer, that is a circular
 *  buffer with a fixed capacity.
 *
 *  \tparam T the type of data to be stored
 *  \tparam CAP the buffers capacity (must be greater than 1)
 */
template <typename T, unsigned CAP>
class BBuffer {
	static_assert(CAP > 1, "BBuffer of size 1 is unsupported.");
	// Prevent copies and assignments
	BBuffer(const BBuffer&)            = delete;
	BBuffer& operator=(const BBuffer&) = delete;

 private:
	T data[CAP];
	volatile unsigned in;
	volatile unsigned out;

 public:
	/*! \brief Constructor that initialized an empty buffer.
	 */
	BBuffer() : in(0), out(0) {}

	/*! \brief Add an element to the buffer.
	 *  \param val The element to be added.
	 *  \return `false` if the buffer is full and no element can be added; `true` otherwise.
	 */
	bool produce(T val) {
		unsigned nextin = (in + 1) % CAP;
		if (nextin != out) {
			data[in] = val;
			in = nextin;
			return true;
		}
		return false;
	}

	/*! \brief Remove an element from the buffer.
	 * \param val Output parameter that receives the next element. If there is
	 *            (currently) no next element, `val` will not be modified.
	 * \return `false` if the buffer was empty; `true` if the buffer was
	 *          not empty and an element was written to val.
	 */
	bool consume(T &val) {
		if (in != out) {
			val = data[out];
			out = (out + 1) % CAP;
			return true;
		}
		return false;
	}
};

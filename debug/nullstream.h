/*! \file
 *  \brief \ref NullStream is a stream discarding everything
 */

#pragma once

#include "object/outputstream.h"

/*! \brief Ignore all data passed by the stream operator
 *  \ingroup io
 *
 * Can be used instead of the \ref OutputStream if (for debugging reasons) all
 * output should be ignored, e.g. for \ref DBG_VERBOSE
 *
 * By using template programming, a single generic methods is sufficient
 * (which simply discard everything).
 */
class NullStream {
 public:
	/*! \brief Empty default constructor
	 */
	NullStream() {}

	/*! \brief Generic stream operator for any data type
	 *
	 * Uses template meta programming for a generic & short solution
	 *
	 * \tparam T Type of data to ignore
	 * \param value data to be ignore
	 * \return Reference to the \ref NullStream object allowing concatenation of operators
	 */
	template <typename T>
	NullStream& operator << (T value) {
		(void) value;
		return *this;
	}
};

extern NullStream nullstream;

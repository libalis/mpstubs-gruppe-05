/*! \file
 *  \brief Template function to determine the length of an array
 */

#pragma once

#include "types.h"

/* \brief Helper to retrieve the number of elements in an array
 * (Warning: template magic)
 * \param Array
 * \return Number of elements
 */
template<class T, size_t N>
constexpr size_t size(T (&/*unused*/)[N]) {
	return N;
}

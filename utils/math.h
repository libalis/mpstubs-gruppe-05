/*! \file
 *  \brief General purpose \ref Math "math functions"
 */

#pragma once

#include "types.h"

/*! \brief Basic math helper functions
 */
namespace Math {
	template <typename T>
	T abs(T a) {
		return (a >= 0 ? a : -a);
	}

	template <typename T>
	T min(T a, T b) {
		return a > b ? b : a;
	}

	template <typename T>
	T max(T a, T b) {
		return a > b ? a : b;
	}
}  // namespace Math

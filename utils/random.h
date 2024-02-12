/*! \file
 *  \brief Class \ref Random (PRNG based on Mersenne Twister)
 */

#pragma once

#include "types.h"

/*! Mersenne Twister (32 bit pseudorandom number generator)
 *
 * \see [Wikipedia](https://en.wikipedia.org/wiki/Mersenne_Twister)
 */
class Random {
 private:
	static const uint32_t N = 624;
	static const uint32_t M = 397;
	static const uint32_t R = 31;
	static const uint32_t A = 0x9908B0DF;

	// initialization multiplier
	static const uint32_t F = 1812433253;

	static const uint32_t U = 11;

	static const uint32_t S = 7;
	static const uint32_t B = 0x9D2C5680;

	static const uint32_t T = 15;
	static const uint32_t C = 0xEFC60000;

	static const uint32_t L = 18;

	static const uint32_t MASK_LOWER = (1UL << R) - 1;
	static const uint32_t MASK_UPPER = (1UL << R);

	uint32_t mt[N];
	uint16_t index;

 public:
	/*! \brief Constructor
	 *
	 *  \param seed initial value (seed) for the pseudorandom number generator.
	 */
	explicit Random(uint32_t seed);

	/*! \brief Get the next (pseudo)random number.
	 *  \return some number
	 */
	uint32_t number();
};

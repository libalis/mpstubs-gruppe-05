#include "utils/random.h"

Random::Random(uint32_t seed) {
	mt[0] = seed;

	for (uint32_t i = 1; i < N; i++) {
		mt[i] = (F * (mt[i - 1] ^ (mt[i - 1] >> 30)) + i);
	}

	index = N;
}

uint32_t Random::number() {
	uint16_t v = index;

	if (index >= N) {
		// Twist
		for (uint32_t i = 0; i < N; i++) {
			uint32_t x = (mt[i] & MASK_UPPER) + (mt[(i + 1) % N] & MASK_LOWER);

			uint32_t xA = x >> 1;

			if ((x & 0x1) != 0) {
				xA ^= A;
			}

			mt[i] = mt[(i + M) % N] ^ xA;
		}
		v = index = 0;
	}

	uint32_t y = mt[v];
	index = v + 1;

	y ^= (y >> U);
	y ^= (y << S) & B;
	y ^= (y << T) & C;
	y ^= (y >> L);

	return y;
}

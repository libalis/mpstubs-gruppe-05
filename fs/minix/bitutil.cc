#include "fs/minix/bitutil.h"
#include "types.h"

static unsigned int find_first_zero(uint16_t u) {
	for (unsigned int i = 0; i < 16; i++) {
		uint16_t bitmask = 1 << i;
		if ((u & bitmask) == 0) {
			return i;
		}
	}
	return 16;
}

unsigned int minix_find_first_zero_bit(void *bitmap, unsigned int size) {
	uint16_t *bm = reinterpret_cast<uint16_t *>(bitmap);

	for (unsigned int n = 0; n < size / 16; n++) {
		if (bm[n] == 0xffff) {
			continue;
		}
		// @Cleanup use ffz intrinsic
		unsigned int i = find_first_zero(bm[n]);
		// we know there is a zero, so we don't check i
		return n * 16 + i;
	}

	return size;
}

bool minix_test_and_set_bit(unsigned int bit, void *bitmap) {
	unsigned int word = bit / 16;
	uint16_t bitmask = 1 << (bit % 16);
	uint16_t *bm = reinterpret_cast<uint16_t *>(bitmap);
	bool was_set = (bm[word] & bitmask) != 0;
	bm[word] |= bitmask;
	return was_set;
}

void minix_set_bit(unsigned int bit, void *bitmap) {
	minix_test_and_set_bit(bit, bitmap);
}

bool minix_test_and_clear_bit(unsigned int bit, void *bitmap) {
	unsigned int word = bit / 16;
	uint16_t bitmask = 1 << (bit % 16);
	uint16_t *bm = reinterpret_cast<uint16_t *>(bitmap);
	bool was_set = (bm[word] & bitmask) != 0;
	bm[word] &= ~bitmask;
	return was_set;
}

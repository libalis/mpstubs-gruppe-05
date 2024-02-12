#pragma once

// little-endian, 16 bit indexed bitmaps

// size is the number of bits in bitmap and must be a multiple of 16
unsigned int minix_find_first_zero_bit(void *bitmap, unsigned int size);
bool minix_test_and_set_bit(unsigned int bit, void *bitmap);
void minix_set_bit(unsigned int bit, void *bitmap);
bool minix_test_and_clear_bit(unsigned int bit, void *bitmap);


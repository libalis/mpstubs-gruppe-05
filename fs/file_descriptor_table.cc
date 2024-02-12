#include "fs/file_descriptor_table.h"

// size is the number of bits in bitmap
static int set_first_zero_bit(void *bitmap, int size) {
	const int bits_per_long = sizeof(long) * 8;  // assume 8 bits per char
	unsigned long *bm = static_cast<unsigned long *>(bitmap);

	int n;
	for (n = 0; n < size / bits_per_long; n++) {
		if (bm[n] == static_cast<unsigned long>(-1)) {
			continue;
		}

		// @Synchronization @Cleanup
		// this loop should do an atomic test and set bit
		for (int i = 0; i < bits_per_long; i++) {
			unsigned long bitmask = 1 << i;
			if ((bm[n] & bitmask) == 0) {
				bm[n] |= bitmask;
				return n * bits_per_long + i;
			}
		}
	}

	// @Synchronization @Cleanup
	// this loop should do an atomic test and set bit
	for (int i = 0; i < size % bits_per_long; i++) {
		unsigned long bitmask = 1 << i;
		if ((bm[n] & bitmask) == 0) {
			bm[n] |= bitmask;
			return n * bits_per_long + i;
		}
	}

	return -1;
}

static void clear_bit(int bit, void *bitmap) {
	const int bits_per_long = sizeof(long) * 8;  // assume 8 bits per char
	unsigned long *bm = static_cast<unsigned long *>(bitmap);

	int n = bit / bits_per_long;
	unsigned long bitmask = 1 << (bit % bits_per_long);

	// @Synchronization atomic and?
	bm[n] &= ~bitmask;
}

bool FD_Table::insert_file(File *file) {
	// @Synchronization
	const int bitmap_bits = sizeof(bitmap) * 8;  // assume 8 bits per char
	int fd = set_first_zero_bit(bitmap, bitmap_bits);
	if (fd < 0) {
		return false;
	}
	file->fd = fd;
	File **bucket = &file_descriptor_table[fd % fd_table_size];
	file->fd_table_next = *bucket;
	*bucket = file;
	return true;
}

File *FD_Table::remove_file(int fd) {
	// @Synchronization
	File **indirect = &file_descriptor_table[fd % fd_table_size];
	File *file = file_descriptor_table[fd % fd_table_size];
	while (file != nullptr) {
		if (file->fd == fd) {
			break;
		}
		indirect = &file->fd_table_next;
		file = file->fd_table_next;
	}
	if (file == nullptr) {
		return nullptr;
	}
	*indirect = file->fd_table_next;
	clear_bit(file->fd, bitmap);
	return file;
}

File *FD_Table::get_file(int fd) {
	if (fd < 0) {
		return nullptr;
	}
	// @Synchronization
	File *file = file_descriptor_table[fd % fd_table_size];
	while (file != nullptr) {
		if (file->fd == fd) {
			return file;
		}
		file = file->fd_table_next;
	}
	return nullptr;
}

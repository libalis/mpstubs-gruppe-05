#include "fs/ramdisk.h"
#include "fs/errno.h"

#include "debug/assert.h"

Block Ramdisk::fix(uint64_t block_number) {
	Block block(this, block_number);
	if ((block_number << blocksize_bits) >> blocksize_bits != block_number ||
		(block_number << blocksize_bits) > size) {
		// FS bug
		block.data = nullptr;
		block.flags = -ENOSPC;
		return block;
	}
	// We can't actually return a pointer to the raw data,
	// because block.forget() doesn't work then. But we only
	// use forget in error cases, so it's fine for now.
	block.data = buf + block_number * blocksize;
	return block;
}

void Ramdisk::unfix(Block *block) {
	block->data = nullptr;
	// noop
}

int Ramdisk::sync() {
	// noop
	return 0;
}

int Ramdisk::sync(Block *block) {
	// noop
	(void)block;
	return 0;
}

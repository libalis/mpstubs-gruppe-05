#include "fs/blockdevice.h"

bool BlockDevice::set_blocksize(unsigned int blocksize) {
	// We could accept arbitrary blocksizes, but filesystems
	// generally only support these.
	if (blocksize == 512) {
		this->blocksize = 512;
		blocksize_bits = 9;
		return true;
	}
	if (blocksize == 1024) {
		this->blocksize = 1024;
		blocksize_bits = 10;
		return true;
	}
	if (blocksize == 2048) {
		this->blocksize = 2048;
		blocksize_bits = 11;
		return true;
	}
	if (blocksize == 4096) {
		this->blocksize = 4096;
		blocksize_bits = 12;
		return true;
	}
	return false;
}

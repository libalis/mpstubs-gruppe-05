#pragma once

#include "fs/blockdevice.h"

class Ramdisk: public BlockDevice {
 private:
	char *buf;
	size_t size;

 public:
	Ramdisk(void *buf, size_t size) : buf(reinterpret_cast<char*>(buf)), size(size) { }
	Block fix(uint64_t block_number);
	void unfix(Block *block);
	int sync();
	int sync(Block *block);
};

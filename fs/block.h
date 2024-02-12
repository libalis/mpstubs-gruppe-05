#pragma once

#include "fs/blockdevice.h"

class Block {
 public:
	static const int DIRTY_FLAG = 0x1;
	uint64_t block_number;
	void *data;
	BlockDevice *bdev;
	// void *bdev_data; private data for the owning block device,
	// could be used to implement a dirty list
	int flags;

	Block(BlockDevice *bdev, uint64_t block_number) {
		this->block_number = block_number;
		data = nullptr;
		this->bdev = bdev;
		flags = 0;
	}

	Block() {
		block_number = -1;
		data = nullptr;
		bdev = nullptr;
		flags = 0;
	}

	void mark_dirty() {
		flags |= DIRTY_FLAG;
	}

	bool is_dirty() const {
		return (flags & DIRTY_FLAG) != 0;
	}

	void clear_dirty() {
		flags &= ~DIRTY_FLAG;
	}

	unsigned int get_size() const {
		return bdev->blocksize;
	}

	void sync() {
		if (is_dirty()) {
			bdev->sync(this);
		}
	}

	void unfix() {
		bdev->unfix(this);
	}

	void forget() {
		// discard changes (currently not supported by ramdisk)
		clear_dirty();
		unfix();
	}
};

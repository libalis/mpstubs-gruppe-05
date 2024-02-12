#pragma once

#include "fs/definitions.h"

class Block;

class BlockDevice {
 public:
	unsigned int blocksize;
	unsigned int blocksize_bits;

	BlockDevice() {
		blocksize = 0;
		blocksize_bits = 0;
	}

	// Fixates a block in memory.
	// (without block cache this is simply a read)
	virtual Block fix(uint64_t block_number) = 0;

	// Releases the block and eventually writes its
	// data to disk if it was marked dirty.
	// (without block cache this is simply a free + write-if-dirty)
	virtual void unfix(Block *block) = 0;

	// If the block is marked dirty,
	// this forces the block's data to be written to disk synchronously.
	virtual int sync(Block *block) = 0;

	// Calls sync on each dirty block.
	// (currently not implemented for lack of a dirty list)
	virtual int sync() = 0;

	// Sets the logical blocksize.
	// This is the number of bytes that will be read/written per 'Block'.
	// Must not match the physical blocksize.
	// Returns false if the requested blocksize is not supported.
	// (Currently 512, 1024, 2048, and 4096 are supported)
	bool set_blocksize(unsigned int blocksize);

	template<typename T>
	T divide_by_blocksize(T val) const {
		return static_cast<T>(val >> blocksize_bits);
	}

	template<typename T>
	T modulo_blocksize(T val) const {
		return static_cast<T>(val & ((1 << blocksize_bits) - 1));
	}

 private:
	BlockDevice(const BlockDevice&)            = delete;
	BlockDevice& operator=(const BlockDevice&) = delete;
};

#include "fs/block.h"

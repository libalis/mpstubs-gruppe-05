#pragma once

#include "fs/definitions.h"

class Filesystem;
class Inode;

class Inode_Cache {
 private:
	static const int soft_limit = 64;  // must be > 0
	static Inode *icache_first;
	static unsigned long num_inodes;

	static void maybe_evict_inodes();

 public:
	static void insert_inode(Inode *inode);
	static Inode *get_inode(Filesystem *fs, ino_t ino);
	static int sync_fs_inodes(Filesystem *fs);

	// Forcefully evicts all inodes that belong to fs.
	// The refcount of all inodes should be zero at this point.
	// If an inode's refcount is not zero, it will be removed
	// from the cache, but it's memory will never be freed.
	static void remove_fs_inodes(Filesystem *fs);
};

#include "fs/filesystem.h"
#include "fs/inode.h"

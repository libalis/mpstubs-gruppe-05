#pragma once

#include "types.h"
#include "fs/filesystem.h"
#include "fs/inode_cache.h"

class Inode {
 public:
	ino_t        ino;
	umode_t      mode;
	unsigned int nlinks;
	uid_t        uid;
	gid_t        gid;
	off_t        size;
	// @Incomplete these are in seconds, add nanoseconds for filesystems that support it
	time_t       atime;
	time_t       mtime;
	time_t       ctime;

	unsigned int refcount;  // When this hits zero the inode cache is free to evict the inode at any time
	Inode *      icache_next;
	Filesystem * filesystem;
	int          flags;

	static const int DIRTY_FLAG = 0x1;
	static const int NEW_FLAG   = 0x2;

	explicit Inode(Filesystem *fs) {
		ino = 0;
		mode = 0;
		nlinks = 0;
		uid = 0;
		gid = 0;
		size = 0;
		atime = 0;
		mtime = 0;
		ctime = 0;
		refcount = 0;
		icache_next = nullptr;
		filesystem = fs;
		flags = NEW_FLAG;

		// Note that the constructor does a get()!
		get();
	}

	// Called when the inode cache decides to evict the inode.
	// You probably want to delete the disk-inode here if inode->nlinks is zero.
	// Or write changes to disk if the inode is dirty.
	virtual ~Inode() {}

	bool is_new() const {
		return (flags & NEW_FLAG) != 0;
	}

	// Inodes are flagged new in the constructor.
	// Once the filesystem has sufficiently initialized the inode
	// for use by others, the new flag should be cleared.
	void clear_new_flag() {
		flags &= ~NEW_FLAG;
	}

	void mark_dirty() {
		flags |= DIRTY_FLAG;
	}

	bool is_dirty() const {
		return (flags & DIRTY_FLAG) != 0;
	}

	int write_to_disk() {
		if (is_dirty()) {
			int error = filesystem->write_inode(this);
			if (error != 0) {
				return error;
			}
			flags &= ~DIRTY_FLAG;
		}
		return 0;
	}

	void get() {
		filesystem->num_inode_references++;
		refcount++;
	}

	void put() {
		if (refcount != 0) {
			filesystem->num_inode_references--;
			refcount--;
		}
		// TODO If the refcount hits zero and nlinks is zero
		// we should remove the inode from the cache immediately.
		// But our cache currently doesn't support that.
	}
};

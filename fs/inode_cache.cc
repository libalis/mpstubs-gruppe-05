#include "fs/inode_cache.h"
#include "fs/errno.h"

// @Speed this is a very slow FIFO cache

Inode *Inode_Cache::icache_first = nullptr;
unsigned long Inode_Cache::num_inodes = 0;

// @Synchronization

void Inode_Cache::insert_inode(Inode *inode) {
	Inode **indirect = &icache_first;
	Inode *cur = icache_first;
	while (cur != nullptr) {
		if (cur->filesystem == inode->filesystem &&
			cur->ino == inode->ino) {
			// fs bug
			return;
		}
		indirect = &cur->icache_next;
		cur = cur->icache_next;
	}

	*indirect = inode;
	inode->icache_next = nullptr;

	num_inodes++;
}

void Inode_Cache::maybe_evict_inodes() {
	Inode **indirect = &icache_first;
	Inode *inode = icache_first;
	while (inode != nullptr) {
		if (num_inodes <= soft_limit) {
			return;
		}

		Inode **next_indirect = &inode->icache_next;
		Inode *next = inode->icache_next;

		if (inode->refcount == 0) {
			*indirect = inode->icache_next;
			next_indirect = indirect;

			delete inode;
			num_inodes--;
		}

		indirect = next_indirect;
		inode = next;
	}
}

Inode *Inode_Cache::get_inode(Filesystem *fs, ino_t ino) {
	maybe_evict_inodes();

	Inode *inode = icache_first;
	while (inode != nullptr) {
		if (inode->ino == ino && inode->filesystem == fs) {
			break;
		}
		inode = inode->icache_next;
	}
	// TODO if the inode's new flag is set, wait until it's clear
	if (inode != nullptr) {
		inode->get();
	} else {
		inode = fs->allocate_inode();
		if (inode == nullptr) {
			return nullptr;
		}
		// we insert the uninitialized inode here, but the new flag is set
		insert_inode(inode);
	}

	return inode;
}

// @Cleanup @Speed Stupid hack to sync inodes, each filesystem should have a dirty list instead.
int Inode_Cache::sync_fs_inodes(Filesystem *fs) {
	Inode **indirect = &icache_first;
	Inode *inode = icache_first;
	while (inode != nullptr) {
		Inode **next_indirect = &inode->icache_next;
		Inode *next = inode->icache_next;
		if (inode->filesystem == fs) {
			if (inode->refcount == 0 && inode->nlinks == 0) {
				*indirect = inode->icache_next;
				next_indirect = indirect;

				delete inode;
				num_inodes--;
			} else {
				if (inode->is_dirty()) {
					inode->write_to_disk();
				}
			}
		}

		indirect = next_indirect;
		inode = next;
	}
	return 0;
}

void Inode_Cache::remove_fs_inodes(Filesystem *fs) {
	Inode **indirect = &icache_first;
	Inode *inode = icache_first;
	while (inode != nullptr) {
		Inode **next_indirect = &inode->icache_next;
		Inode *next = inode->icache_next;
		if (inode->filesystem == fs) {
			*indirect = inode->icache_next;
			next_indirect = indirect;

			if (inode->refcount == 0) {
				delete inode;
			}
			num_inodes--;
		}

		indirect = next_indirect;
		inode = next;
	}
}

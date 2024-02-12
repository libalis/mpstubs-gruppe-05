// SPDX-License-Identifier: GPL-2.0

#include "fs/minix/minix.h"
#include "fs/minix/bitutil.h"
#include "fs/errno.h"
#include "utils/string.h"
#include "debug/output.h"

int Minix::new_block() {
	unsigned int bits_per_zone = 8 * bdev->blocksize;

	for (int i = 0; i < super->zmap_blocks; i++) {
		Block *block = &zmap[i];

		// spin_lock(&bitmap_lock);
		unsigned int j = minix_find_first_zero_bit(block->data, bits_per_zone);
		if (j < bits_per_zone) {
			minix_set_bit(j, block->data);
			// spin_unlock(&bitmap_lock);
			block->mark_dirty();
			j += i * bits_per_zone + super->firstdatazone - 1;
			if (j < super->firstdatazone || j >= super->nzones) {
				break;
			}
			return j;
		}
		// spin_unlock(&bitmap_lock);
	}
	return 0;
}

Minix_Disk_Inode *Minix::raw_inode(ino_t ino, Block *block, int *error) {
	int blockno;
	int minix_inodes_per_block = bdev->blocksize / sizeof(Minix_Disk_Inode);

	if (ino == 0 || ino > super->ninodes) {
		*error = -EINVAL;
		return nullptr;
	}
	ino--;
	blockno = 2 + super->imap_blocks + super->zmap_blocks + ino / minix_inodes_per_block;
	*block = bdev->fix(blockno);
	if (block->data == nullptr) {
		*error = block->flags;
		return nullptr;
	}
	Minix_Disk_Inode *disk_inode = reinterpret_cast<Minix_Disk_Inode *>(block->data);
	return disk_inode + ino % minix_inodes_per_block;
}

Inode *Minix::new_inode(umode_t mode, int *error) {
	// @Synchronization
	unsigned int bits_per_zone = 8 * bdev->blocksize;

	// find and allocate free space in the inode bitmap
	unsigned long j = bits_per_zone;
	Block *block;
	// spin_lock(&bitmap_lock);
	int i;
	for (i = 0; i < super->imap_blocks; i++) {
		block = &imap[i];
		if (block->data == nullptr) {
			*error = block->flags;
			return nullptr;
		}
		j = minix_find_first_zero_bit(block->data, bits_per_zone);
		if (j < bits_per_zone) {
			break;
		}
	}
	*error = -ENOSPC;
	if (j >= bits_per_zone) {
		// spin_unlock(&bitmap_lock);
		return nullptr;
	}
	if (minix_test_and_set_bit(j, block->data)) { /* shouldn't happen */
		// spin_unlock(&bitmap_lock);
		return nullptr;
	}
	// spin_unlock(&bitmap_lock);
	unsigned long bit = j;
	j += i * bits_per_zone;
	if (j == 0 || j > super->ninodes) {
		return nullptr;
	}

	// allocate and initialize the new inode (@Incomplete)
	Inode *inode = allocate_inode();
	if (inode == nullptr) {
		minix_test_and_clear_bit(bit, block->data);
		*error = -ENOMEM;
		return nullptr;
	}
	block->mark_dirty();

	// inode_init_owner(inode, dir, mode);
	inode->ino = j;
	inode->mode = mode;
	// inode->i_mtime = inode->i_atime = inode->i_ctime = current_time(inode);
	// inode->i_blocks = 0;
	memset(minix_i(inode)->data, 0, sizeof(minix_i(inode)->data));
	inode->mark_dirty();

	// inode is now fully initialized
	// @Cleanup @Incomplete no, it is not. Look at how Linux uses the 'new flag'
	inode->clear_new_flag();
	Inode_Cache::insert_inode(inode);

	*error = 0;
	return inode;
}

/* Clear the link count and mode of a deleted inode on disk. */
void Minix::clear_disk_inode(Inode *inode) {
	int error;
	Block block;
	Minix_Disk_Inode *disk_inode = raw_inode(inode->ino, &block, &error);
	if (disk_inode == nullptr) {
		return;  // nothing we can do about that
	}
	disk_inode->nlinks = 0;
	disk_inode->mode = 0;
	block.mark_dirty();
	block.unfix();
}

void Minix::free_inode(Inode *inode) {
	int k = bdev->blocksize_bits + 3;

	unsigned long ino = inode->ino;
	if (ino < 1 || ino > super->ninodes) {
#ifdef VERBOSE
		DBG_VERBOSE << "minix_free_inode: inode 0 or nonexistent inode" << endl;
#endif
		return;
	}
	// @Cleanup
	unsigned long bit = ino & ((1 << k) - 1);
	ino >>= k;
	if (ino >= super->imap_blocks) {
#ifdef VERBOSE
		DBG_VERBOSE << "minix_free_inode: nonexistent imap in superblock" << endl;
#endif
		return;
	}

	clear_disk_inode(inode); /* clear on-disk copy */

	Block *block = &imap[ino];
	// @Synchronization
	// spin_lock(&bitmap_lock);
	if (!minix_test_and_clear_bit(bit, block->data)) {
#ifdef VERBOSE
		DBG_VERBOSE << "minix_free_inode: bit already cleared" << endl;
#endif
	}
	// spin_unlock(&bitmap_lock);
	block->mark_dirty();
}

void Minix::free_block(unsigned long block) {
	int k = bdev->blocksize_bits + 3;

	if (block < super->firstdatazone || block >= super->nzones) {
#ifdef VERBOSE
		DBG_VERBOSE << "Trying to free block not in datazone" << endl;
#endif
		return;
	}
	unsigned long zone = block - super->firstdatazone + 1;
	// @Cleanup what even is this?
	unsigned long bit = zone & ((1 << k) - 1);
	zone >>= k;
	if (zone >= super->zmap_blocks) {
#ifdef VERBOSE
		DBG_VERBOSE << "minix_free_block: nonexistent bitmap buffer" << endl;
#endif
		return;
	}
	Block *b = &zmap[zone];
	// @Synchronization
	// spin_lock(&bitmap_lock);
	if (!minix_test_and_clear_bit(bit, b->data)) {
#ifdef VERBOSE
		DBG_VERBOSE << "minix_free_block: bit already cleared" << endl;
#endif
	}
	// spin_unlock(&bitmap_lock);
	b->mark_dirty();
}

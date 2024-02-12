// SPDX-License-Identifier: GPL-2.0

#include "fs/minix/minix.h"
#include "fs/errno.h"
#include "utils/alloc.h"
#include "utils/string.h"
#include "fs/minix/bitutil.h"
#include "debug/assert.h"

Inode *Minix::allocate_inode() {
	MinixInode *inode = new MinixInode(this);
	return static_cast<Inode *>(inode);
}

Inode *Minix::iget(unsigned long ino, int *error) {
	Inode *inode = Inode_Cache::get_inode(this, ino);
	if (inode == nullptr) {
		*error = -ENOMEM;
		return nullptr;
	}

	if (!inode->is_new()) {
		*error = 0;
		return inode;
	}

	MinixInode *minix_inode = minix_i(inode);
	Block block;
	Minix_Disk_Inode *disk_inode = raw_inode(ino, &block, error);
	if (disk_inode == nullptr) {
		delete inode;
		return nullptr;
	}
	inode->ino    = ino;
	inode->mode   = disk_inode->mode;
	inode->nlinks = disk_inode->nlinks;
	inode->uid    = disk_inode->uid;
	inode->gid    = disk_inode->gid;
	inode->size   = disk_inode->size;
	inode->atime  = disk_inode->atime;
	inode->mtime  = disk_inode->mtime;
	inode->ctime  = disk_inode->ctime;
	for (int i = 0; i < 10; i++) {
		minix_inode->data[i] = disk_inode->zone[i];
	}
	block.unfix();
	inode->clear_new_flag();  // inode is now fully initialized
	return inode;
}

int Minix::write_inode(Inode *inode) {
	Block block;
	int error = 0;
	Minix_Disk_Inode *disk_inode = raw_inode(inode->ino, &block, &error);
	if (disk_inode == nullptr) {
		return error;
	}
	disk_inode->mode   = inode->mode;
	disk_inode->nlinks = inode->nlinks;
	disk_inode->uid    = inode->uid;
	disk_inode->gid    = static_cast<unsigned char>(inode->gid);
	disk_inode->size   = inode->size;
	disk_inode->atime  = inode->atime;
	disk_inode->mtime  = inode->mtime;
	disk_inode->ctime  = inode->ctime;

	MinixInode *minix_inode = minix_i(inode);
	for (int i = 0; i < 10; i++) {
		disk_inode->zone[i] = minix_inode->data[i];
	}
	block.mark_dirty();
	block.unfix();
	return 0;
}

int Minix::mount(const void *data) {
	unsigned long i;
	unsigned long blockno;
	int ret = 0;

	(void)data;  // @Incomplete mount options?

	if (!bdev->set_blocksize(BLOCK_SIZE)) {
		return -EINVAL;
	}

	super_block = bdev->fix(1);
	if (super_block.data == nullptr) {
		return super_block.flags;
	}

	super = reinterpret_cast<Minix_Super_Block *>(super_block.data);
	if (super->magic != MINIX3_SUPER_MAGIC) {
		bdev->unfix(&super_block);
		return -EINVAL;
	}

	if (!bdev->set_blocksize(super->blocksize)) {
		bdev->unfix(&super_block);
		return -EINVAL;
	}

	/*
	 * Allocate the buffer map to keep the superblock small.
	 */
	if (super->imap_blocks == 0 || super->zmap_blocks == 0) {
		bdev->unfix(&super_block);
		return -EINVAL;
	}

	size_t size = (super->imap_blocks + super->zmap_blocks) * sizeof(Block);
	Block *map = reinterpret_cast<Block *>(malloc(size));
	if (map == nullptr) {
		bdev->unfix(&super_block);
		return -ENOMEM;
	}
	memset(reinterpret_cast<void*>(map), 0, size);
	imap = &map[0];
	zmap = &map[super->imap_blocks];

	blockno = 2;
	for (i = 0; i < super->imap_blocks; i++) {
		imap[i] = bdev->fix(blockno);
		if (imap[i].data == nullptr) {
			ret = imap[i].flags;
			goto error;
		}
		blockno++;
	}
	for (i = 0; i < super->zmap_blocks; i++) {
		zmap[i] = bdev->fix(blockno);
		if (zmap[i].data == nullptr) {
			ret = zmap[i].flags;
			goto error;
		}
		blockno++;
	}

	minix_set_bit(0, imap[0].data);
	minix_set_bit(0, zmap[0].data);

	// @Incomplete We don't check whether the filesystem was created correctly.

	root_inode = iget(MINIX_ROOT_INO, &ret);
	if (root_inode == nullptr) {
		goto error;
	}
	if (!S_ISDIR(root_inode->mode)) {
		root_inode->put();
		ret = -EINVAL;
		goto error;
	}

	return 0;

error:
	umount();
	return ret;
}

void Minix::umount() {
	for (unsigned long i = 0; i < super->imap_blocks; i++) {
		Block *block = &imap[i];
		if (block->data == nullptr) {
			// this is only for the mount error case
			break;
		}
		bdev->unfix(block);
	}
	for (unsigned long i = 0; i < super->zmap_blocks; i++) {
		Block *block = &zmap[i];
		if (block->data == nullptr) {
			// this is only for the mount error case
			break;
		}
		bdev->unfix(block);
	}
	free(imap);
	bdev->unfix(&super_block);
}

int Minix::sync() {
	// Note that we never write to the super block
	// so there is no need to sync it

	for (uint16_t i = 0; i < super->imap_blocks; i++) {
		imap[i].sync();
	}
	for (uint16_t i = 0; i < super->zmap_blocks; i++) {
		zmap[i].sync();
	}
	return 0;
}

int Minix::open(File *file) {
	// noop
	(void)file;
	return 0;
}

int Minix::close(File *file) {
	// noop
	(void)file;
	return 0;
}

int Minix::link(Inode *dir, const char *filename, size_t name_len, Inode *inode) {
	if (name_len > 60) {
		return -ENAMETOOLONG;
	}

	int error = add_link(dir, inode, filename, name_len);
	if (error != 0) {
		return error;
	}

	inode->nlinks++;
	inode->mark_dirty();

	return 0;
}

Inode *Minix::create(Inode *dir, const char *filename, size_t name_len, umode_t mode, int *error) {
	if (name_len > 60) {
		*error = -ENAMETOOLONG;
		return nullptr;
	}
	Inode *inode = new_inode(mode, error);
	if (inode == nullptr) {
		return nullptr;
	}
	*error = add_link(dir, inode, filename, name_len);
	if (*error != 0) {
		inode->nlinks = 0;
		inode->mark_dirty();
		inode->put();
		return nullptr;
	}
	inode->nlinks = 1;
	inode->mark_dirty();
	return inode;
}

int Minix::symlink(Inode *dir, const char *filename, size_t name_len, const char *symname) {
	if (name_len > 60) {
		return -ENAMETOOLONG;
	}
	size_t len = strlen(symname) + 1;
	if (len > bdev->blocksize || len > MAX_SYMLINK_LEN) {
		return -ENAMETOOLONG;
	}
	int error = 0;
	Inode *inode = new_inode(S_IFLNK | 0777, &error);
	if (inode == nullptr) {
		return error;
	}
	inode->nlinks = 0;
	// I guess we have to loop here?? write should never return 0
	size_t bytes_written = 0;
	do {
		ssize_t retval = write(inode, symname + bytes_written, len - bytes_written,
		                       bytes_written);
		if (retval < 0) {
			inode->mark_dirty();
			inode->put();
			return retval;
		}
		bytes_written += retval;
	} while (bytes_written < len);

	error = add_link(dir, inode, filename, name_len);
	if (error == 0) {
		inode->nlinks = 1;
		inode->size = len;
	}
	inode->mark_dirty();
	inode->put();
	return error;
}

int Minix::unlink(Inode *dir, const char *filename, size_t name_len, Inode *inode) {
	int error = delete_entry(dir, filename, name_len);
	if (error != 0) {
		return error;
	}

	inode->nlinks--;
	inode->mark_dirty();

	return 0;
}

int Minix::rename(Inode *old_dir, Inode *old_inode, const char *old_name, size_t old_name_len,
                  Inode *new_dir, Inode *new_inode, const char *new_name, size_t new_name_len) {
	if (old_name_len > 60 || new_name_len > 60) {
		return -ENAMETOOLONG;  // shortcut
	}
	int error = 0;
	Block old_block;
	Minix_Dirent *old_dirent = find_dirent(old_dir, old_name, old_name_len, &old_block, &error);
	if (old_dirent == nullptr) {
		if (error == 0) {
			error = -ENOENT;
		}
		return error;
	}
	bool old_is_dir = S_ISDIR(old_inode->mode);

	Block new_block;
	Minix_Dirent *new_dirent = find_dirent(new_dir, new_name, new_name_len, &new_block, &error);
	if (new_dirent == nullptr && error != 0) {
		goto out_old_put_unfix;
	}

	if (new_dirent != 0) {
		assert(new_inode);
		bool new_is_dir = S_ISDIR(new_inode->mode);
		// @Cleanup I think VFS does most of these checks now (except the empty check)
		if (old_is_dir) {
			if (!new_is_dir) {
				error = -ENOTDIR;
				goto out_new_put_unfix;
			}
			error = check_dir_is_empty(new_inode);
			if (error != 0) {
				goto out_new_put_unfix;
			}
		} else if (new_is_dir) {
			error = -EISDIR;
			goto out_new_put_unfix;
		}

		new_dirent->inode = old_inode->ino;
		new_block.mark_dirty();
		new_block.unfix();

		// new_dir->i_mtime = new_dir->i_ctime = current_time(new_dir);
		// new_inode->i_ctime = current_time(new_inode);
		if (new_is_dir) {
			new_inode->nlinks--;  // twice because of ..
		}
		new_inode->nlinks--;
		new_inode->mark_dirty();
		new_inode->put();
	} else {
		assert(!new_inode);
		error = add_link(new_dir, old_inode, new_name, new_name_len);
		if (error != 0) {
			goto out_old_put_unfix;
		}
		if (old_is_dir) {
			new_dir->nlinks++;  // .. will now point to new_dir
		}
	}
	old_inode->put();

	old_dirent->inode = 0;  // delete the old entry
	old_block.mark_dirty();
	old_block.unfix();
	// old_dir->i_ctime = old_dir->i_mtime = current_time(old_dir);
	// old_dir->mark_dirty();

	if (old_is_dir) {
		// fix .. of the directory we moved
		uint64_t blockno = get_block(old_inode, 0, false, &error);
		if (error != 0) {
			return error;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}
		Minix_Dirent *dirent = reinterpret_cast<Minix_Dirent *>(block.data);
		dirent++;  // .. is always the second dirent
		assert(strcmp(dirent->name, "..") == 0);
		dirent->inode = new_dir->ino;
		block.mark_dirty();
		block.unfix();

		old_dir->nlinks--;  // .. no longer points to old_dir
		old_dir->mark_dirty();
	}

	assert(error == 0);
	assert(!old_block.data);
	assert(!new_block.data);
	return 0;

out_new_put_unfix:
	new_inode->put();
	new_block.unfix();
out_old_put_unfix:
	old_inode->put();
	old_block.unfix();
	return error;
}

#include "fs/minix/minix.h"
#include "fs/errno.h"
#include "utils/string.h"
#include "debug/assert.h"

/*
 * Return the offset into block `lblock' of the last valid
 * byte in that block, plus one.
 */
unsigned int Minix::dir_block_last_byte(Inode *dir, unsigned long lblock) {
	unsigned int last_byte = bdev->blocksize;
	unsigned long size = static_cast<unsigned long>(dir->size);

	if (lblock == bdev->divide_by_blocksize(size)) {
		last_byte = bdev->modulo_blocksize(size);
	}

	if (last_byte % sizeof(Minix_Dirent) != 0) {
		assert(false);  // this shouldn't happen unless the filesystem is corrupted
		last_byte -= last_byte % sizeof(Minix_Dirent);
	}

	return last_byte;
}

static inline bool namecompare(size_t len, const char *name, const char *buffer) {
	if (len < 60 && buffer[len] != '\0') {
		return false;
	}
	return strncmp(name, buffer, len) == 0;
}

// Does not set error to -ENOENT if no dirent was found. Make sure to unfix the block.
Minix_Dirent *Minix::find_dirent(Inode *dir, const char *filename, size_t name_len,
                                 Block *p_block, int *error) {
	// @Cleanup this could call iterate_dir with its own context
	*error = 0;
	if (name_len > 60) {
		return nullptr;
	}

	unsigned long size = dir->size;
	unsigned long num_blocks = bdev->divide_by_blocksize(size + bdev->blocksize - 1);

	// lblock = logical block
	for (unsigned long lblock = 0; lblock < num_blocks; lblock++) {
		uint64_t blockno = get_block(dir, lblock, false, error);
		if (*error != 0) {
			return nullptr;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			*error = block.flags;
			return nullptr;
		}
		char *data = reinterpret_cast<char *>(block.data);
		unsigned int last_byte = dir_block_last_byte(dir, lblock);
		Minix_Dirent *limit = reinterpret_cast<Minix_Dirent *>(data + last_byte) - 1;
		for (Minix_Dirent *dirent = reinterpret_cast<Minix_Dirent *>(data); dirent <= limit; dirent++) {
			if (dirent->inode == NULL) {
				continue;
			}
			if (namecompare(name_len, filename, dirent->name)) {
				*p_block = block;
				return dirent;
			}
		}
		block.unfix();
	}
	return nullptr;
}

Inode *Minix::lookup(Inode *dir, const char *filename, size_t name_len, int *error) {
	Block block;
	Minix_Dirent *dirent = find_dirent(dir, filename, name_len, &block, error);
	if (dirent == nullptr) {
		return nullptr;
	}
	uint32_t ino = dirent->inode;
	block.unfix();
	return iget(ino, error);
}

static inline size_t namelen(const char *buffer) {
	size_t len = 60;
	while (buffer[len - 1] == 0) {
		len--;
		if (len == 0) {
			// shouldn't happen
			break;
		}
	}
	return len;
}

int Minix::iterate_dir(Inode *dir, Dir_Context *ctx) {
	unsigned long size = dir->size;
	unsigned long num_blocks = bdev->divide_by_blocksize(size + bdev->blocksize - 1);

	if (ctx->pos % sizeof(Minix_Dirent) != 0) {
		ctx->pos -= ctx->pos % sizeof(Minix_Dirent);  // add or subtract or fail?
	}

	unsigned long lblock = bdev->divide_by_blocksize(ctx->pos);  // logical block
	unsigned int offset_in_block = bdev->modulo_blocksize(ctx->pos);

	for (; lblock < num_blocks; lblock++) {
		int error = 0;
		uint64_t blockno = get_block(dir, lblock, false, &error);
		if (error != 0) {
			return error;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}
		char *data = reinterpret_cast<char *>(block.data);

		Minix_Dirent *limit = reinterpret_cast<Minix_Dirent *>(data + dir_block_last_byte(dir, lblock)) - 1;

		Minix_Dirent *dirent = reinterpret_cast<Minix_Dirent *>(data + offset_in_block);
		for (; dirent <= limit; dirent++) {
			if (dirent->inode != 0) {
				size_t len = namelen(dirent->name);

				unsigned char type = DT_UNKNOWN;
				Inode * inode = iget(dirent->inode, &error);
				if (inode != nullptr && error == 0) {
					type = (inode->mode >> 12) & 15;
					inode->put();
				}

				if (!ctx->dir_emit(dirent->name, len,
							dirent->inode, type)) {
					block.unfix();
					return 0;
				}
			}
			ctx->pos += sizeof(Minix_Dirent);
		}
		block.unfix();
		offset_in_block = 0;
	}
	return 0;
}

int Minix::add_link(Inode *dir, Inode *inode, const char *name, size_t name_len) {
	Minix_Dirent *dirent;
	Block block;

	if (name_len > 60) {
		return -ENAMETOOLONG;
	}

	// search for null entries first, otherwise append
	for (unsigned long lblock = 0; ; lblock++) {  // lblock = logical block
		int error;
		uint64_t blockno = get_block(dir, lblock, true, &error);
		if (error != 0) {
			return error;
		}
		block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}

		char *data = reinterpret_cast<char *>(block.data);
		Minix_Dirent *dir_end = reinterpret_cast<Minix_Dirent *>(data + dir_block_last_byte(dir, lblock));

		Minix_Dirent *limit = reinterpret_cast<Minix_Dirent *>(data + bdev->blocksize) - 1;
		for (dirent = reinterpret_cast<Minix_Dirent *>(data); dirent <= limit; dirent++) {
			if (dirent == dir_end) {
				dirent->inode = 0;
				dir->size += sizeof(Minix_Dirent);
				goto got_it;
			}

			if (dirent->inode == 0) {
				goto got_it;
			}

			if (namecompare(name_len, name, dirent->name)) {
				block.unfix();
				return -EEXIST;
			}
		}

		block.unfix();
	}
	assert(false);  // we either return an error or jump to got_it

got_it:
	dirent->inode = inode->ino;
	memcpy(dirent->name, name, name_len);
	memset(dirent->name + name_len, 0, 60 - name_len);
	block.mark_dirty();
	block.unfix();
	// dir->i_mtime = dir->i_ctime = current_time(dir);
	dir->mark_dirty();
	return 0;
}

int Minix::make_empty_dir(Inode *inode, Inode *parent_dir) {
	// one block is always enough for two dirents
	int error;
	// @Cleanup should really make a helper function
	// for getting a logical block
	uint64_t blockno = get_block(inode, 0, true, &error);
	if (error != 0) {
		return error;
	}
	Block block = bdev->fix(blockno);
	if (block.data == nullptr) {
		return block.flags;
	}

	Minix_Dirent *dirent = reinterpret_cast<Minix_Dirent *>(block.data);

	dirent->inode = inode->ino;
	memset(dirent->name, 0, sizeof(dirent->name));
	dirent->name[0] = '.';

	dirent++;

	dirent->inode = parent_dir->ino;
	memset(dirent->name, 0, sizeof(dirent->name));
	dirent->name[0] = '.';
	dirent->name[1] = '.';

	inode->nlinks++;
	inode->size = 2 * sizeof(Minix_Dirent);
	inode->mark_dirty();

	parent_dir->nlinks++;
	parent_dir->mark_dirty();

	block.mark_dirty();
	block.unfix();
	return 0;
}

int Minix::mkdir(Inode *parent_dir, const char *filename, size_t name_len, umode_t mode) {
	if (name_len > 60) {
		return -ENAMETOOLONG;
	}
	int error;
	Inode *inode = new_inode(mode, &error);
	if (inode == nullptr) {
		return error;
	}
	inode->nlinks = 0;
	error = make_empty_dir(inode, parent_dir);
	if (error != 0) {
		goto error;
	}
	error = add_link(parent_dir, inode, filename, name_len);
	if (error != 0) {
		goto error;
	}
	inode->nlinks++;
	inode->mark_dirty();
	inode->put();
	return 0;

error:
	inode->nlinks = 0;  // this deletes the inode (eventually)
	inode->mark_dirty();
	inode->put();
	return error;
}

int Minix::delete_entry(Inode *dir, const char *filename, size_t name_len) {
	int error = 0;
	Block block;
	Minix_Dirent *dirent = find_dirent(dir, filename, name_len, &block, &error);
	if (dirent == nullptr) {
		if (error == 0) {
			error = -ENOENT;
		}
		return error;
	}
	// we just set the inode number to 0, directories never get smaller
	dirent->inode = 0;
	block.mark_dirty();
	block.unfix();
	return 0;
}

// routine to check that the specified directory is empty (for rmdir)
// returns 0 if empty or -ENOTEMPTY otherwise
int Minix::check_dir_is_empty(Inode *dir) {
	// @Cleanup this could call iterate_dir with its own context

	unsigned long size = dir->size;
	unsigned long num_blocks = bdev->divide_by_blocksize(size + bdev->blocksize - 1);

	// lblock = logical block
	for (unsigned long lblock = 0; lblock < num_blocks; lblock++) {
		int error;
		uint64_t blockno = get_block(dir, lblock, false, &error);
		if (error != 0) {
			return error;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}
		char *data = reinterpret_cast<char *>(block.data);

		unsigned int last_byte = dir_block_last_byte(dir, lblock);
		Minix_Dirent *limit = reinterpret_cast<Minix_Dirent *>(data + last_byte) - 1;
		for (Minix_Dirent *dirent = reinterpret_cast<Minix_Dirent *>(data); dirent <= limit; dirent++) {
			if (dirent->inode == 0) {
				continue;
			}
			// @Cleanup
			// @Cleanup

			/* check for . and .. */
			char *name = dirent->name;
			if (name[0] != '.') {
				goto not_empty;
			}
			if (name[1] == '\0') {
				if (dirent->inode != dir->ino) {
					goto not_empty;
				}
			} else if (name[1] != '.' || name[1] == '\0') {
				goto not_empty;
			}

			continue;

		not_empty:
			block.unfix();
			return -ENOTEMPTY;
		}
		block.unfix();
	}

	return 0;
}

int Minix::rmdir(Inode *parent_dir, const char *filename, size_t name_len, Inode *dir) {
	int error = check_dir_is_empty(dir);
	if (error != 0) {
		return error;
	}

	error = delete_entry(parent_dir, filename, name_len);
	if (error != 0) {
		return error;
	}
	parent_dir->nlinks--;  // '..' of subdir gets deleted
	parent_dir->mark_dirty();
	dir->nlinks = 0;  // this deletes the inode (eventually)
	dir->mark_dirty();

	return 0;
}

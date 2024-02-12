#include "fs/filesystem.h"
#include "fs/errno.h"
#include "fs/util.h"
#include "fs/definitions.h"
#include "utils/alloc.h"

static size_t minsize(size_t a, size_t b) {
	return a < b ? a : b;
}

ssize_t Filesystem::read(Inode *inode, void *buf, size_t count, off_t pos) {
	Filesystem *fs = inode->filesystem;
	BlockDevice *bdev = fs->bdev;
	unsigned int blocksize = bdev->blocksize;
	uint64_t logical_block = bdev->divide_by_blocksize(pos);
	unsigned int offset_in_block = bdev->modulo_blocksize(pos);

	char *to = reinterpret_cast<char *>(buf);
	size_t bytes_read = 0;
	while (bytes_read < count) {
		if (pos + bytes_read >= static_cast<size_t>(inode->size)) {
			break;
		}
		int error = 0;
		uint64_t blockno = fs->get_block(inode, logical_block, false, &error);
		if (error != 0) {
			return error;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}

		size_t bytes_left = inode->size - pos - bytes_read;
		size_t remaining_in_block = blocksize - offset_in_block;
		size_t bytes_to_read = count - bytes_read;

		size_t n = minsize(bytes_left, bytes_to_read);
		n = minsize(remaining_in_block, n);
		char *from = reinterpret_cast<char *>(block.data) + offset_in_block;
		if (copy_to_user(to + bytes_read, from, n) != n) {
			block.unfix();
			return -EFAULT;
		}

		block.unfix();
		bytes_read += n;
		logical_block++;
		offset_in_block = 0;
	}
	return bytes_read;
}

ssize_t Filesystem::write(Inode *inode, const void *buf, size_t count, off_t pos) {
	Filesystem *fs = inode->filesystem;
	BlockDevice *bdev = fs->bdev;
	unsigned int blocksize = bdev->blocksize;
	uint64_t logical_block = bdev->divide_by_blocksize(pos);
	unsigned int offset_in_block = bdev->modulo_blocksize(pos);

	char *from = reinterpret_cast<char *>(const_cast<void*>(buf));
	size_t bytes_written = 0;
	while (bytes_written < count) {
		int error = 0;
		uint64_t blockno = fs->get_block(inode, logical_block, true, &error);
		if (error != 0) {
			return error;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}

		unsigned int remaining = blocksize - offset_in_block;
		size_t n = minsize(remaining, count - bytes_written);
		char *to = reinterpret_cast<char *>(block.data) + offset_in_block;
		if (copy_from_user(to, from + bytes_written, n) != n) {
			block.unfix();
			return -EFAULT;
		}

		block.mark_dirty();
		block.unfix();

		bytes_written += n;
		logical_block++;
		offset_in_block = 0;
	}
	return bytes_written;
}

int Filesystem::punch_hole(Inode *inode, off_t from, off_t to) {
	Filesystem *fs = inode->filesystem;
	BlockDevice *bdev = fs->bdev;
	unsigned int blocksize = bdev->blocksize;
	uint64_t logical_block = bdev->divide_by_blocksize(from);
	unsigned int offset_in_block = bdev->modulo_blocksize(from);

	// @Incomplete on error truncate to old size
	while (from < to) {
		int error = 0;
		uint64_t blockno = fs->get_block(inode, logical_block, true, &error);
		if (error != 0) {
			return error;
		}
		Block block = bdev->fix(blockno);
		if (block.data == nullptr) {
			return block.flags;
		}

		unsigned int remaining_in_block = blocksize - offset_in_block;
		size_t remaining = to - from;
		size_t n = minsize(remaining_in_block, remaining);
		char *dest = reinterpret_cast<char *>(block.data) + offset_in_block;
		memset(dest, 0, n);

		block.mark_dirty();
		block.unfix();

		from += n;
		logical_block++;
		offset_in_block = 0;
	}

	return 0;
}

static void symlink_buffer_cleanup(const char *buf) {
	free(reinterpret_cast<void *>(const_cast<char*>(buf)));
}

const char *Filesystem::get_link(Inode *inode, void (**cleanup_callback)(const char *buf), int *error) {
	if (inode->size == 0) {
		// apparently Posix allows empty symlinks
		return "";
	}
	if (inode->size > MAX_SYMLINK_LEN) {
		*error = -ENAMETOOLONG;
		return nullptr;
	}
	char *buf = reinterpret_cast<char *>(malloc(inode->size + 1));
	if (buf == nullptr) {
		*error = -ENOMEM;
		return nullptr;
	}
	// I guess we have to loop here?? read should never return 0
	ssize_t bytes_read = 0;
	do {
		ssize_t retval = read(inode, buf + bytes_read, inode->size - bytes_read, bytes_read);
		if (retval < 0) {
			free(buf);
			*error = retval;
			return nullptr;
		}
		bytes_read += retval;
	} while (bytes_read < inode->size);

	// null-terminate for filesystems that don't store a null-terminated string for symlinks
	buf[inode->size] = 0;
	*cleanup_callback = symlink_buffer_cleanup;
	return buf;
}

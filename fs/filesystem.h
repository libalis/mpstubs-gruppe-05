#pragma once

#include "fs/blockdevice.h"
#include "fs/dir_context.h"
#include "fs/definitions.h"

class Inode;
class File;

typedef void (*symlink_cleanup_callback_t)(const char *symlink_buffer);

class Filesystem {
 public:
	BlockDevice *bdev;
	Inode *root_inode;
	unsigned long num_inode_references;  // number of get()s - number of put()s on inodes of this fs

	Filesystem() {
		bdev = nullptr;
		root_inode = nullptr;
		num_inode_references = 0;
	}

	virtual ~Filesystem() {}

	// After a successful mount the root_inode should be initialized.
	virtual int mount(const void *data) = 0;

	// For filesystems that need additional data on open files.
	virtual int open(File *file) = 0;
	virtual int close(File *file) = 0;

	// Returns the physical block number (the one you pass to the fix function of the blockdevice,
	// for the blockdevice this is the logical block number) of the block that contains the bytes
	// [logical_block * blocksize, (logical_block + 1) * blocksize) of the inode.
	// If create is true the block will be created if it doesn't exist.
	// On error a negative errno is written into error.
	virtual uint64_t get_block(Inode *inode, uint64_t logical_block, bool create, int *error) = 0;

	// Usually filesystems do not need to implement this!
	// A generic implementation that uses the filesystem's get_block function is provided
	// as default in filesystem.cc.
	// buf can be but must not be a user buffer.
	virtual ssize_t read(Inode *inode, void *buf, size_t count, off_t pos);

	// Usually filesystems do not need to implement this!
	// A generic implementation that uses the filesystem's get_block function is provided
	// as default in filesystem.cc.
	// buf can be but must not be a user buffer.
	virtual ssize_t write(Inode *inode, const void *buf, size_t count, off_t pos);

	// Initializes bytes [from, to) of the inode to zero.
	// Filesystems that support fancy sparse files might want to implement this,
	// otherwise a generic implementation that uses the filesystem's get_block function
	// is provided as default in filesystem.cc.
	virtual int punch_hole(Inode *inode, off_t from, off_t to);

	// Returns a buffer that contains the zero-terminated path of a symlink.
	// The callback will be called with that buffer as the argument to free any
	// resources associated with the buffer. This is necessary since some filesystems
	// (e.g. ext2/3/4) have "fast symlinks" where the symlink path is stored inline with
	// the inode, so no allocations and therefore no freeing is needed. In that case just
	// don't set the callback.
	// The default implementation in filesystem.cc allocates a buffer with size 'inode->size + 1',
	// uses the read function to fill the buffer and zero-terminates it.
	// Symlinks must not be longer than MAX_SYMLINK_LEN (see definitions.h)
	// Returns nullptr and sets 'error' on error.
	virtual const char *get_link(Inode *inode, void (**cleanup_callback)(const char *buf), int *error);

	// Truncates the size of the inode to 'length' bytes.
	virtual void truncate(Inode *inode, off_t length) = 0;

	// Calls ctx->dir_emit for each entry in 'directory' starting at ctx->pos.
	// See also dir_context.h.
	virtual int iterate_dir(Inode *directory, Dir_Context *ctx) = 0;

	// Creates a new regular file.
	// Sets error to -EEXIST if a file with that name already exists in directory.
	// Returns nullptr and sets 'error' on error.
	virtual Inode *create(Inode *directory, const char *filename, size_t name_len,
			umode_t mode, int *error) = 0;

	// Creates a new hard link.
	// Returns -EEXIST if a file with that name already exists in directory.
	virtual int link(Inode *directory, const char *filename, size_t name_len, Inode *inode) = 0;

	// Creates a new symlink. symname is a user buffer.
	// Returns -EEXIST if a file with that name already exists in directory.
	virtual int symlink(Inode *directory, const char *filename, size_t name_len,
	                    const char *symname) = 0;

	// Removes a link (if it exists).
	virtual int unlink(Inode *directory, const char *filename, size_t name_len, Inode *inode) = 0;

	// Searches the given directory for a file with the given name and
	// returns its inode.
	// Returns nullptr if the filename was not found. (Does not set error to -ENOENT.)
	// Returns nullptr and sets 'error' on error.
	virtual Inode *lookup(Inode *directory, const char *filename, size_t name_len, int *error) = 0;

	// Creates a new empty directory.
	// Returns -EEXIST if a file with that name already exists in directory.
	virtual int mkdir(Inode *parent_dir, const char *filename, size_t name_len, umode_t mode) = 0;

	// Removes an empty directory.
	// (Returns -ENOTEMPTY if the directory is not empty).
	virtual int rmdir(Inode *parent_dir, const char *filename, size_t name_len, Inode *dir) = 0;

	// Moves a link from old_dir to new_dir (might be the same) and changes its name from
	// old_name to new_name (might be the same only if old_dir != new_dir).
	// The VFS does NOT check whether the old link actually exists since the filesystem
	// has to try and access it anyway, so if it doesn't exist return -ENOENT. If the old
	// link points to a directory and the new link already exists, then it has to be a
	// directory as well and it has to be empty, otherwise return -ENOTDIR or -ENOTEMPTY.
	virtual int rename(Inode *old_dir, Inode *old_inode, const char *old_name, size_t old_name_len,
	                   Inode *new_dir, Inode *new_inode, const char *new_name, size_t new_name_len) = 0;

	// Filesystems can allocate their own subclass of Inode with this.
	virtual Inode *allocate_inode() = 0;

	// Used to write the changes of a dirty inode to disk.
	virtual int write_inode(Inode *inode) = 0;

	// Should synchronously commit all cached changes to disk.
	virtual int sync() = 0;

	// Sync gets called before this, just free all memory and unfix all blocks here
	virtual void umount() = 0;
};

#include "fs/inode.h"
#include "fs/file.h"

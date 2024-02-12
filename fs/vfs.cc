#include "fs/errno.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/minix/minix.h"
#include "fs/dir_context.h"
#include "fs/util.h"
#include "utils/alloc.h"
#include "utils/string.h"
#include "debug/output.h"

/* TODO:
 * -symbolic links
 */

Filesystem *VFS::root_fs = nullptr;
// @Incomplete these should be per process
Inode *VFS::global_cwd = nullptr;
FD_Table VFS::fd_table;

struct path {
	Inode *cur_dir;
	const char *pathname;
};

// @Incomplete mount should take a string instead of a BlockDevice *
int VFS::mount(const char *fstype, BlockDevice *bdev, const void *data) {
	/* Mount BlockDevice `*bdev` depending on fstype and register it */

	if (root_fs != nullptr) {
		// at the moment only support for one mounted filesystem
		return -ENOSYS;
	}

	if (strcmp(fstype, "minix") != 0) {
		return -ENODEV;
	}

	Filesystem *file_system = new Minix();
	if (file_system == nullptr) {
		return -ENOMEM;
	}
	file_system->bdev = bdev;

	int error = file_system->mount(data);
	if (error != 0) {
		delete file_system;
		return error;
	}
	if (file_system->root_inode == nullptr || !S_ISDIR(file_system->root_inode->mode)) {
		delete file_system;
		return -EINVAL;
	}

	root_fs = file_system;
	return 0;
}

int VFS::sync_fs(Filesystem *fs) {
	int error = Inode_Cache::sync_fs_inodes(fs);
	if (error != 0) {
		return error;
	}

	error = fs->sync();
	if (error != 0) {
		return error;
	}

	error = fs->bdev->sync();
	return error;
}

int VFS::umount() {
	// @Synchronization
	Filesystem *fs = root_fs;
	if (fs == nullptr) {
		return -EINVAL;
	}

	// @Synchronization need exclusive access to the filesystem

	// We always have a reference to our root inode and
	// we might have an additional reference if the current working directory
	// belongs to this filesystem.
	if (fs->num_inode_references > 2 ||
	    (fs->num_inode_references == 2 && global_cwd->filesystem != fs)) {
		return -EBUSY;
	}

	int error = sync_fs(fs);
	if (error != 0) {
		return error;
	}

	if (global_cwd->filesystem == fs) {
		global_cwd->put();
		global_cwd = nullptr;
	}
	fs->root_inode->put();
	Inode_Cache::remove_fs_inodes(fs);

	fs->umount();

	delete fs;
	root_fs = nullptr;

	return 0;
}

void VFS::sync() {
	// @Synchronization
	Filesystem *fs = root_fs;
	if (fs == nullptr) {
		return;
	}

	sync_fs(fs);
}

// @Incomplete mode
int VFS::open(const char *pathname, int flags) {
	if (pathname == nullptr) {
		return -EINVAL;
	}
	Filesystem *fs = root_fs;
	if (fs == nullptr) {
		return -ENODEV;
	}

	const int supported_flags = O_ACCMODE | O_CREAT | O_EXCL;
	if ((flags & ~supported_flags) != 0) {
		// @Incomplete
		return -EINVAL;
	}

	if ((flags & O_EXCL) != 0 && (flags & O_CREAT) == 0) {
		return -EINVAL;
	}
	int accmode = flags & O_ACCMODE;

	struct path path;
	int error = pathwalk_step12(&path, pathname, global_cwd, 0);
	if (error != 0) {
		return error;
	}
	const char *filename = path.pathname;
	Inode *parent_dir = path.cur_dir;
	bool must_be_dir = strchr(filename, '/') != nullptr;
	if (must_be_dir) {
		if ((flags & O_CREAT) != 0) {
			// open cannot create directories
			parent_dir->put();
			return -EINVAL;
		}
	}

	parent_dir->get();  // Step 3 will decrement the refcount but we might still need this for create
	Inode *inode = pathwalk_step3(&path, true, 0, &error);
	if (inode != nullptr) {
		parent_dir->put();
		if ((flags & O_EXCL) != 0) {
			inode->put();
			return -EEXIST;
		}
		if (must_be_dir && !S_ISDIR(inode->mode)) {
			inode->put();
			return -ENOENT;
		}
		if (S_ISDIR(inode->mode) && (accmode == O_WRONLY || accmode == O_RDWR)) {
			// directories cannot be opened with write access
			inode->put();
			return -EISDIR;
		}
	} else {
		if (error != 0 && error != -ENOENT) {
			parent_dir->put();
			return error;
		}

		if ((flags & O_CREAT) == 0) {
			parent_dir->put();
			return -ENOENT;
		}

		// at this point filename cannot refer to a directory anymore (no trailing slashes)

		// @Incomplete mode is always 0777
		umode_t mode = 0777 | S_IFREG;
		// @Synchronization
		inode = fs->create(parent_dir, filename, strlen(filename), mode, &error);
		parent_dir->put();
		if (inode == nullptr) {
			return error;
		}
	}

	// we finally have our inode, now create the file
	File *file = new File(inode, accmode);
	if (file == nullptr) {
		inode->put();
		return -ENOMEM;
	}
	int retval = fs->open(file);
	if (retval != 0) {
		delete file;
		return retval;
	}

	if (!fd_table.insert_file(file)) {
		delete file;
		return -EMFILE;  // or is it ENFILE??
	}
	return file->fd;
}

int VFS::close(int fd) {
	// @Synchronization Probably need a refcount on files
	// or just a lock, not sure what the posix semantics for close are

	// We always close the file descriptor even if fs->close fails,
	// because Linux does it like that aswell
	File *file = fd_table.remove_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	Filesystem *fs = file->inode->filesystem;
	int error = fs->close(file);

	delete file;
	return error;
}

// TODO check for integer overflows/underflows

ssize_t VFS::read(int fd, void *buf, size_t count) {
	// @Synchronization
	if (buf == nullptr || count == 0) {
		return 0;
	}

	if (count > SSIZE_MAX) {
		count = SSIZE_MAX;
	}

	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	if ((file->accmode & O_WRONLY) != 0) {
		return -EBADF;
	}

	Inode *inode = file->inode;
	if (!S_ISREG(inode->mode)) {
		// @Incomplete there are only regular files and directories currently
		return -EISDIR;
	}

	Filesystem *fs = inode->filesystem;
	// @Synchronization
	ssize_t bytes_read = fs->read(inode, buf, count, file->pos);
	if (bytes_read < 0) {
		// error case, don't update the file position
		return bytes_read;
	}

	file->pos += bytes_read;
	return bytes_read;
}

ssize_t VFS::write(int fd, const void *buf, size_t count) {
	// @Synchronization
	if ((buf == nullptr) || count == 0) {
		return 0;
	}

	if (count > SSIZE_MAX) {
		count = SSIZE_MAX;
	}

	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	if ((file->accmode & O_RDONLY) != 0) {
		return -EBADF;
	}

	Inode *inode = file->inode;
	if (!S_ISREG(inode->mode)) {
		// @Incomplete there are only regular files and directories currently
		// Note that write access to a directory shouldn't even be possible
		return -EISDIR;
	}

	off_t original_size = inode->size;
	Filesystem *fs = inode->filesystem;

	// @Incomplete check against fs max_filesize

	if (file->pos > inode->size) {
		// @Synchronization
		int error = fs->punch_hole(inode, inode->size, file->pos);
		if (error != 0) {
			fs->truncate(inode, original_size);  // ignore errors while we try to recover
			return error;
		}
	}

	// @Synchronization
	ssize_t bytes_written = fs->write(inode, buf, count, file->pos);
	// Not sure what should happen for bytes_written == 0.
	// I have no idea how that would even happen or what should happen
	// if we already punched a hole. I guess we just keep the hole and return 0.
	if (bytes_written < 0) {
		fs->truncate(inode, original_size);  // ignore errors while we try to recover
		return bytes_written;
	}

	file->pos += bytes_written;
	if (file->pos > inode->size) {
		// @Cleanup here we change the size in the VFS
		// but for truncate we do it in the filesystem.
		inode->size = file->pos;
		inode->mark_dirty();
	}
	return bytes_written;
}

off_t VFS::lseek(int fd, off_t offset, int whence) {
	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	// @Synchronization
	switch (whence) {
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += file->pos;
			break;
		case SEEK_END:
			offset += file->inode->size;
			break;
		default:
			return -EINVAL;
	}

	file->pos = offset;
	return offset;
}

int VFS::stat(Inode *inode, struct stat *statbuf) {
	// @Incomplete always sets 0 for st_dev and st_rdev
	// @Synchronization
	struct stat sb = {};
	sb.st_dev = 0;
	sb.st_ino = inode->ino;
	sb.st_mode = inode->mode;
	sb.st_nlink = inode->nlinks;
	sb.st_uid = inode->uid;
	sb.st_gid = inode->gid;
	sb.st_rdev = 0;
	sb.st_size = inode->size;
	sb.st_blksize = inode->filesystem->bdev->blocksize;
	sb.st_blocks = (inode->size + 511) >> 9;
	sb.st_atime = inode->atime;
	sb.st_mtime = inode->mtime;
	sb.st_ctime = inode->ctime;

	if (copy_to_user(statbuf, &sb, sizeof(sb)) != sizeof(sb)) {
		return -EFAULT;
	}

	return 0;
}

int VFS::stat(const char *pathname, struct stat *statbuf) {
	int error = 0;
	Inode *inode = pathwalk_step123(pathname, global_cwd, true, 0, &error);
	if (inode == nullptr) {
		return error;
	}
	error = stat(inode, statbuf);
	inode->put();
	return error;
}

int VFS::lstat(const char *pathname, struct stat *statbuf) {
	int error = 0;
	// don't follow a final symlink here
	Inode *inode = pathwalk_step123(pathname, global_cwd, false, 0, &error);
	if (inode == nullptr) {
		return error;
	}
	error = stat(inode, statbuf);
	inode->put();
	return error;
}

int VFS::fstat(int fd, struct stat *statbuf) {
	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}
	return stat(file->inode, statbuf);
}

int VFS::getdents(int fd, Dirent *dirp, int count) {
	if (count < 0 || count < static_cast<int>(sizeof(Dirent)) + 2) {
		return -EINVAL;
	}

	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	Inode *inode = file->inode;
	if (!S_ISDIR(inode->mode)) {
		return -ENOTDIR;
	}

	if (file->pos >= inode->size) {
		return 0;
	}

	Filesystem *fs = inode->filesystem;
	Readdir_Context ctx(file->pos, dirp, count);
	// @Synchronization
	int error = fs->iterate_dir(inode, &ctx);
	if (error != 0) {
		return error;
	}

	if (ctx.error != 0) {
		return ctx.error;
	}

	file->pos = ctx.pos;
	return ctx.buf_used;
}

DIR * VFS::opendir(const char *name) {
	struct stat statbuf = {};
	if (name != nullptr  && stat(name, &statbuf) >= 0 && S_ISDIR(statbuf.st_mode)) {
		int fd = open(name, O_RDONLY);
		lseek(fd, 0, SEEK_SET);
		if (fd >= 0) {
			DIR *dirp = reinterpret_cast<DIR *>(calloc(1, sizeof(DIR)));
			if (dirp == nullptr) {
				close(fd);
			} else {
				dirp->fd = fd;
				dirp->offset = 0;
				dirp->size = 0;
				return dirp;
			}
		}
	}
	return 0;
}

Dirent * VFS::readdir(DIR *dirp) {
	if (dirp == 0) {
		return 0;
	} else if (dirp->offset >= dirp->size) {
		int len = getdents(dirp->fd, reinterpret_cast<Dirent *>(dirp->buf), sizeof(dirp->buf));
		if (len <= 0) {
			return 0;
		}
		dirp->size = len;
		dirp->offset = 0;
	}
	Dirent * e = reinterpret_cast<Dirent *>(dirp->buf + dirp->offset);
	dirp->offset += e->d_reclen;
	return e;
}

void VFS::rewinddir(DIR *dirp) {
	if (dirp != 0) {
		lseek(dirp->fd, 0, SEEK_SET);
		dirp->size = 0;
		dirp->offset = 0;
	}
}

int VFS::closedir(DIR *dirp) {
	if (dirp != 0) {
		int r = close(dirp->fd);
		free(dirp);
		return r;
	} else {
		return 0;
	}
}

int VFS::mkdir(const char *pathname) {
	Filesystem *fs = root_fs;
	if (fs == nullptr) {
		return -ENODEV;
	}

	struct path path;
	int error = pathwalk_step12(&path, pathname, global_cwd, 0);
	if (error != 0) {
		return error;
	}
	const char *filename = path.pathname;
	Inode *parent_dir = path.cur_dir;
	size_t name_len = strchrnul(filename, '/') - filename;

	// @Incomplete mode is always 0777
	umode_t mode = 0777 | S_IFDIR;
	// @Synchronization
	error = fs->mkdir(parent_dir, filename, name_len, mode);
	parent_dir->put();
	return error;
}

int VFS::chdir(Inode *inode) {
	if (!S_ISDIR(inode->mode)) {
		return -ENOTDIR;
	}

	inode->get();  // increment refcount, because we want to keep our cwd around
	// @Synchronization race between reading + 'get'ing and setting + 'put'ing cwd
	if (global_cwd != nullptr) {
		global_cwd->put();
	}
	global_cwd = inode;
	return 0;
}

int VFS::chdir(const char *path) {
	int error = 0;
	Inode *inode = pathwalk_step123(path, global_cwd, true, 0, &error);
	if (inode == nullptr) {
		return error;
	}

	error = chdir(inode);
	inode->put();
	return error;
}

int VFS::fchdir(int fd) {
	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	return chdir(file->inode);
}

int VFS::truncate(Inode *inode, off_t length) {
	if (S_ISDIR(inode->mode)) {
		return -EISDIR;
	}

	if (length == inode->size) {
		return 0;
	}

	Filesystem *fs = inode->filesystem;
	if (length > inode->size) {
		// @Synchronization
		return fs->punch_hole(inode, inode->size, length);
	}

	// @Synchronization
	fs->truncate(inode, length);
	return 0;
}

int VFS::truncate(const char *path, off_t length) {
	int error = 0;
	Inode *inode = pathwalk_step123(path, global_cwd, true, 0, &error);
	if (inode == nullptr) {
		return error;
	}

	error = truncate(inode, length);
	inode->put();
	return error;
}

int VFS::ftruncate(int fd, off_t length) {
	File *file = fd_table.get_file(fd);
	if (file == nullptr) {
		return -EBADF;
	}

	if ((file->accmode & O_RDONLY) != 0) {
		return -EBADF;
	}

	return truncate(file->inode, length);
}

ssize_t VFS::readlink(const char *pathname, char *buf, size_t bufsiz) {
	if (bufsiz == 0) {
		return -EINVAL;
	}
	int error = 0;
	Inode *inode = pathwalk_step123(pathname, global_cwd, false, 0, &error);
	if (inode == nullptr) {
		return error;
	}
	if (!S_ISLNK(inode->mode)) {
		inode->put();
		return -EINVAL;
	}
	void (*cleanup_callback)(const char *buf) = nullptr;
	Filesystem *fs = inode->filesystem;
	const char *link_path = fs->get_link(inode, &cleanup_callback, &error);
	inode->put();
	if (link_path == nullptr) {
		return error;
	}

	size_t retval = strlen(link_path);
	if (copy_to_user(buf, link_path, retval) != retval) {
		retval = -EFAULT;
	}

	if (cleanup_callback != nullptr) {
		cleanup_callback(link_path);
	}

	return retval;
}

int VFS::link(const char *oldpath, const char *newpath) {
	int error = 0;
	// See the NOTES in the manpage about following a symlink.
	// Spoiler: Since POSIX.1-2008 it is "implementation-dependent".
	// We don't follow the symlink since otherwise there would be no
	// way to create additional hardlinks to a symlink.
	Inode *inode = pathwalk_step123(oldpath, global_cwd, false, 0, &error);
	if (inode == nullptr) {
		return error;
	}

	if (S_ISDIR(inode->mode)) {
		inode->put();
		return -EPERM;
	}

	// @Incomplete check inode->nlinks against fs max_links

	struct path path;
	error = pathwalk_step12(&path, newpath, global_cwd, 0);
	if (error != 0) {
		inode->put();
		return error;
	}
	const char *filename = path.pathname;
	Inode *dir = path.cur_dir;

	if (strchr(filename, '/') != nullptr) {
		// Note that Linux handles trailing slashes in newpath very differently.
		// If the directory exists Linux returns -EEXIST, otherwise -ENOENT.
		// I don't know why I should do another lookup here so I just return -EINVAL.
		// Could even do this earlier but who cares about this case anyway.
		// The manpage doesn't really say anything about this.
		dir->put();
		inode->put();
		return -EINVAL;
	}

	Filesystem *fs = dir->filesystem;

	// @Synchronization
	error = fs->link(dir, filename, strlen(filename), inode);
	dir->put();
	inode->put();
	return error;
}

int VFS::symlink(const char *target, const char *linkpath) {
	struct path path;
	int error = pathwalk_step12(&path, linkpath, global_cwd, 0);
	if (error != 0) {
		return error;
	}
	const char *filename = path.pathname;
	Inode *dir = path.cur_dir;

	if (strchr(filename, '/') != nullptr) {
		// Note that Linux handles trailing slashes in newpath very differently.
		// If the directory exists Linux returns -EEXIST, otherwise -ENOENT.
		// I don't know why I should do another lookup here so I just return -EINVAL.
		// Could even do this earlier but who cares about this case anyway.
		// The manpage doesn't really say anything about this.
		dir->put();
		return -EINVAL;
	}

	Filesystem *fs = dir->filesystem;

	// @Synchronization
	error = fs->symlink(dir, filename, strlen(filename), target);
	dir->put();
	return error;
}

int VFS::unlink(const char *pathname) {
	struct path path;
	int error = pathwalk_step12(&path, pathname, global_cwd, 0);
	if (error != 0) {
		return error;
	}
	const char *filename = path.pathname;
	Inode *dir = path.cur_dir;

	if (strchr(filename, '/') != nullptr) {
		// Apparently this isn't Posix behavior but Linux does it this way?
		// Whatever, I don't know what else I'm supposed to do with trailing slashes.
		dir->put();
		return -EISDIR;
	}

	size_t name_len = strlen(filename);
	Filesystem *fs = dir->filesystem;
	// @Speed could let the filesystem do the lookup since it has to access the directory entry
	//        anyway in unlink but this is more convenient for the filesystem
	// @Synchronization
	Inode *inode = fs->lookup(dir, filename, name_len, &error);
	if (inode == nullptr) {
		dir->put();
		return error == 0 ? -ENOENT : error;
	}
	if (S_ISDIR(inode->mode)) {
		dir->put();
		inode->put();
		return -EISDIR;
	}

	// @Synchronization
	error = fs->unlink(dir, filename, name_len, inode);
	dir->put();
	inode->put();
	return error;
}

int VFS::rmdir(const char *pathname) {
	struct path path;
	int error = pathwalk_step12(&path, pathname, global_cwd, 0);
	if (error != 0) {
		return error;
	}
	const char *filename = path.pathname;
	Inode *dir = path.cur_dir;

	size_t name_len = strchrnul(filename, '/') - filename;
	Filesystem *fs = dir->filesystem;
	// @Speed could let the filesystem do the lookup since it has to access the directory entry
	//        anyway in unlink but this is more convenient for the filesystem
	// @Synchronization
	Inode *inode = fs->lookup(dir, filename, name_len, &error);
	if (inode == nullptr) {
		dir->put();
		return error == 0 ? -ENOENT : error;
	}
	if (!S_ISDIR(inode->mode)) {
		dir->put();
		inode->put();
		return -EISDIR;
	}

	// @Synchronization
	error = fs->rmdir(dir, filename, name_len, inode);  // NOLINT
	dir->put();
	inode->put();
	return error;
}

int VFS::rename(const char *oldpath, const char *newpath) {
	// @Incomplete have to check whether an attempt is made to make a directory
	// a subdirectory of itself (return EINVAL, see manpage).
	// This would probably make the pathwalk much more complicated...
	// Looking at the strings won't suffice because of symlinks!

	struct path old_path;
	int error = pathwalk_step12(&old_path, oldpath, global_cwd, 0);
	if (error != 0) {
		return error;
	}
	const char *old_name = old_path.pathname;
	Inode *old_dir = old_path.cur_dir;

	struct path new_path;
	error = pathwalk_step12(&new_path, newpath, global_cwd, 0);
	if (error != 0) {
		old_dir->put();
		return error;
	}
	const char *new_name = new_path.pathname;
	Inode *new_dir = new_path.cur_dir;

	Filesystem *fs = old_dir->filesystem;
	if (new_dir->filesystem != fs) {
		old_dir->put();
		new_dir->put();
		return -EXDEV;
	}

	// @Speed we often walk over a string multiple times like here
	bool old_must_be_dir = strchr(old_name, '/') != nullptr;
	bool new_must_be_dir = strchr(new_name, '/') != nullptr;
	size_t old_name_len = strchrnul(old_name, '/') - old_name;
	size_t new_name_len = strchrnul(new_name, '/') - new_name;

	// @Speed doing the lookups in the filesystem would be faster (see rmdir comment)
	Inode *old_inode = fs->lookup(old_dir, old_name, old_name_len, &error);
	if (old_inode == nullptr) {
		old_dir->put();
		new_dir->put();
		return error == 0 ? -ENOENT : error;
	}
	bool old_is_dir = S_ISDIR(old_inode->mode);
	if (old_must_be_dir && !old_is_dir) {
		old_inode->put();
		old_dir->put();
		new_dir->put();
		return -ENOENT;  // not sure this is the correct errno
	}

	Inode *new_inode = fs->lookup(new_dir, new_name, new_name_len, &error);
	if (new_inode == nullptr && error != 0) {
		old_inode->put();
		old_dir->put();
		new_dir->put();
		return error;
	}

	if (new_inode != nullptr) {
		bool new_is_dir = S_ISDIR(new_inode->mode);
		// I love Posix
		if ((new_must_be_dir && !new_is_dir) || (!new_is_dir && old_is_dir)) {
			error = -ENOTDIR;
		} else if (new_is_dir && !old_is_dir) {
			error = -EISDIR;
		}
		if (error != 0) {
			old_inode->put();
			new_inode->put();
			old_dir->put();
			new_dir->put();
			return error;
		}
	}

	// is this the right time for this check?
	if (old_dir != new_dir || old_name_len != new_name_len ||
	    strncmp(old_name, new_name, old_name_len) != 0) {
		// @Synchronization
		error = fs->rename(old_dir, old_inode, old_name, old_name_len,
		                   new_dir, new_inode, new_name, new_name_len);
	}
	old_inode->put();
	if (new_inode != nullptr) {
		new_inode->put();
	}
	old_dir->put();
	new_dir->put();
	return error;
}

static inline const char *skip_slashes(const char *s) {
	while (*s == '/') {
		s++;
	}
	return s;
}

// Step 1 of the path resolution process described in 'man path_resolution',
// initializes the path struct for step 2.
int VFS::pathwalk_step1(struct path *path, const char *pathname, Inode *cwd) {
	Filesystem *fs = root_fs;
	if (fs == nullptr) {
		return -ENODEV;
	}

	if (pathname[0] == 0) {
		return -ENOENT;
	} else if (pathname[0] == '/') {
		// absolute path, start at root_fs->root_inode
		path->cur_dir = fs->root_inode;
		path->pathname = skip_slashes(pathname);
	} else {
		// relative path
		// @Synchronization race between reading + 'get'ing and setting + 'put'ing cwd
		path->cur_dir = cwd != nullptr ? cwd : fs->root_inode;
		path->pathname = pathname;
	}

	path->cur_dir->get();
	return 0;
}

// Step 2 of the path resolution process described in 'man path_resolution',
// resolves the path up to (excluding) the last component.
int VFS::pathwalk_step2(struct path *path, int depth) {
	for (;;) {
		const char *slash = strchr(path->pathname, '/');
		if (slash == nullptr) {
			break;
		}
		const char *next = skip_slashes(slash);
		if (next[0] == 0) {
			break;
		}
		size_t name_len = static_cast<size_t>(slash - path->pathname);
		if (name_len == 1 && path->pathname[0] == '.') {
			path->pathname = next;
			continue;
		}
		Filesystem *fs = path->cur_dir->filesystem;
		if (name_len == 2 && path->pathname[0] == '.' && path->pathname[1] == '.') {
			// @Speed handle .. completely without the filesystem
			if (path->cur_dir == fs->root_inode) {
				// handle this case because I'm not sure every filesystem
				// is prepared for it
				path->pathname = next;
				continue;
			}
		}
		// @Synchronization
		int error = 0;
		Inode *inode = fs->lookup(path->cur_dir, path->pathname, name_len, &error);
		if (inode == nullptr) {
			path->cur_dir->put();
			return error == 0 ? -ENOENT : error;
		}
		if (S_ISLNK(inode->mode)) {
			Inode *symlink = inode;
			inode = resolve_symlink(symlink, path->cur_dir, depth, &error);
			symlink->put();
			if (inode == nullptr) {
				path->cur_dir->put();
				return error;
			}
		}
		path->cur_dir->put();
		if (!S_ISDIR(inode->mode)) {
			inode->put();
			return -ENOTDIR;
		}
		path->cur_dir = inode;
		path->pathname = next;
	}
	return 0;
}

// Step 3 of the path resolution process described in 'man path_resolution',
// resolves the last component. Does NOT set error to -ENOENT if the inode wasn't found.
Inode *VFS::pathwalk_step3(struct path *path, bool follow_symlink, int depth, int *error) {
	Filesystem *fs = path->cur_dir->filesystem;
	bool must_be_dir;
	size_t name_len;
	const char *slash = strchr(path->pathname, '/');
	if (slash != nullptr) {
		must_be_dir = true;
		name_len = static_cast<size_t>(slash - path->pathname);
	} else {
		must_be_dir = false;
		name_len = strlen(path->pathname);
	}
	// @Speed handle .. without the filesystem aswell
	if (name_len == 0 || (name_len == 1 && path->pathname[0] == '.')) {
		// I think name_len == 0 happens only if the whole path is '/'
		// or maybe if symlinks are involved.
		// Posix says a path that ends with '/' is equivalent to the same
		// path with '/.' at the end.
		return path->cur_dir;
	}
	// @Synchronization
	Inode *inode = fs->lookup(path->cur_dir, path->pathname, name_len, error);
	if (inode == nullptr) {
		path->cur_dir->put();
		*error = -ENOENT;
		return nullptr;
	}
	if (follow_symlink && S_ISLNK(inode->mode)) {
		Inode *symlink = inode;
		inode = resolve_symlink(symlink, path->cur_dir, depth, error);
		symlink->put();
		if (inode == nullptr) {
			path->cur_dir->put();
			return nullptr;
		}
	}
	path->cur_dir->put();
	if (must_be_dir && !S_ISDIR(inode->mode)) {
		inode->put();
		*error = -ENOENT;
		return nullptr;
	}
	path->cur_dir = nullptr;
	path->pathname = nullptr;
	return inode;
}

int VFS::pathwalk_step12(struct path *path, const char *pathname, Inode *cwd, int depth) {
	int error = pathwalk_step1(path, pathname, cwd);
	if (error != 0) {
		return error;
	}
	error = pathwalk_step2(path, depth);
	return error;
}

Inode *VFS::pathwalk_step23(struct path *path, bool follow_final_symlink, int depth, int *error) {
	*error = pathwalk_step2(path, depth);
	if (*error != 0) {
		return nullptr;
	}
	return pathwalk_step3(path, follow_final_symlink, depth, error);
}

Inode *VFS::pathwalk_step123(const char *pathname, Inode *cwd, bool follow_final_symlink,
                             int depth, int *error) {
	struct path path;
	*error = pathwalk_step12(&path, pathname, cwd, depth);
	if (*error != 0) {
		return nullptr;
	}
	return pathwalk_step3(&path, follow_final_symlink, depth, error);
}

Inode *VFS::resolve_symlink(Inode *symlink, Inode *cur_dir, int depth, int *error) {
	if (depth >= MAX_DEPTH) {
		*error = -ELOOP;
		return nullptr;
	}
	Filesystem *fs = symlink->filesystem;
	void (*cleanup_callback)(const char *buf) = nullptr;
	const char *link_path = fs->get_link(symlink, &cleanup_callback, error);
	if (link_path == nullptr) {
		return nullptr;
	}
	Inode *inode = pathwalk_step123(link_path, cur_dir, true, depth + 1, error);
	if (cleanup_callback != nullptr) {
		cleanup_callback(link_path);
	}
	return inode;
}

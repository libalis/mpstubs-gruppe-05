/*! \file
 *  \brief \ref GuardedVFS, a \ref Guarded "guarded" interface for \ref VFS "Virtual File System (VFS)"
 */

#pragma once

#include "fs/vfs.h"
#include "interrupt/guarded.h"

/*! \brief \ref Guarded interface to the \ref VFS used by user applications.
 *
 * Implements the system call interface for class \ref VFS. All methods
 * provided by this class are wrappers for the respective method from the base
 * class, which provide additional synchronization by using the class \ref Guarded.
 */
class GuardedVFS : public VFS {
 public:
	/*! \copydoc VFS::mount()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int mount(const char *fstype, BlockDevice *bdev, const void *data) {
		Guarded section;
		return VFS::mount(fstype, bdev, data);
	}

	/*! \copydoc VFS::umount()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int umount() {
		Guarded section;
		return VFS::umount();
	}

	/*! \copydoc VFS::sync()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static void sync() {
		Guarded section;
		return VFS::sync();
	}

	/*! \copydoc VFS::open()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int open(const char *pathname, int flags) {
		Guarded section;
		return VFS::open(pathname, flags);
	}

	/*! \copydoc VFS::close()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int close(int fd) {
		Guarded section;
		return VFS::close(fd);
	}

	/*! \copydoc VFS::read()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static ssize_t read(int fd, void *buf, size_t count) {
		Guarded section;
		return VFS::read(fd, buf, count);
	}

	/*! \copydoc VFS::write()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static ssize_t write(int fd, const void *buf, size_t count) {
		Guarded section;
		return VFS::write(fd, buf, count);
	}

	/*! \copydoc VFS::lseek()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static off_t lseek(int fd, off_t offset, int whence) {
		Guarded section;
		return VFS::lseek(fd, offset, whence);
	}

	/*! \copydoc VFS::truncate()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int truncate(const char *path, off_t length) {
		Guarded section;
		return VFS::truncate(path, length);
	}

	/*! \copydoc VFS::ftruncate()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int ftruncate(int fd, off_t length) {
		Guarded section;
		return VFS::ftruncate(fd, length);
	}

	/*! \copydoc VFS::link()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int link(const char *oldpath, const char *newpath) {
		Guarded section;
		return VFS::link(oldpath, newpath);
	}

	/*! \copydoc VFS::symlink()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int symlink(const char *target, const char *linkpath) {
		Guarded section;
		return VFS::symlink(target, linkpath);
	}

	/*! \copydoc VFS::unlink()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int unlink(const char *pathname) {
		Guarded section;
		return VFS::unlink(pathname);
	}

	/*! \copydoc VFS::rmdir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int rmdir(const char *pathname) {
		Guarded section;
		return VFS::rmdir(pathname);
	}

	/*! \copydoc VFS::rename()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int rename(const char *oldpath, const char *newpath)  {
		Guarded section;
		return VFS::rename(oldpath, newpath);
	}

	/*! \copydoc VFS::stat()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int stat(const char *pathname, struct stat *statbuf) {
		Guarded section;
		return VFS::stat(pathname, statbuf);
	}

	/*! \copydoc VFS::lstat()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int lstat(const char *pathname, struct stat *statbuf) {
		Guarded section;
		return VFS::lstat(pathname, statbuf);
	}

	/*! \copydoc VFS::fstat()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int fstat(int fd, struct stat *statbuf) {
		Guarded section;
		return VFS::fstat(fd, statbuf);
	}

	/*! \copydoc VFS::readlink()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
		Guarded section;
		return VFS::readlink(pathname, buf, bufsiz);
	}

	/*! \copydoc VFS::getdents()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int getdents(int fd, Dirent *dirp, int count) {
		Guarded section;
		return VFS::getdents(fd, dirp, count);
	}

	/*! \copydoc VFS::opendir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static DIR * opendir(const char *name) {
		Guarded section;
		return VFS::opendir(name);
	}

	/*! \copydoc VFS::readdir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static struct Dirent * readdir(DIR *dirp) {
		Guarded section;
		return VFS::readdir(dirp);
	}

	/*! \copydoc VFS::rewinddir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static void rewinddir(DIR *dirp) {
		Guarded section;
		return VFS::rewinddir(dirp);
	}

	/*! \copydoc VFS::closedir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int closedir(DIR *dirp) {
		Guarded section;
		return VFS::closedir(dirp);
	}

	/*! \copydoc VFS::mkdir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int mkdir(const char *pathname) {
		Guarded section;
		return VFS::mkdir(pathname);
	}

	/*! \copydoc VFS::chdir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int chdir(const char *path) {
		Guarded section;
		return VFS::chdir(path);
	}

	/*! \copydoc VFS::fchdir()
	 *
	 * \note This method is equal to the correspondent method in base class
	 *       \ref VFS, with the only difference that the call will be protected
	 *       by a \ref Guarded object.
	 */
	static int fchdir(int fd) {
		Guarded section;
		return VFS::fchdir(fd);
	}
};

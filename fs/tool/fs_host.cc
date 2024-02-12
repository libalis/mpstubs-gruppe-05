#include "fs/tool/fs_host.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

int FileSystemHost::chdir(const char *path) {
	return ::chdir(path);
}

int FileSystemHost::readlink(const char *pathname, char *buf, unsigned bufsiz) {
	return ::readlink(pathname, buf, bufsiz);
}

int FileSystemHost::open(const char *pathname, bool write) {
	return ::open(pathname, write ? (O_CREAT | O_WRONLY | O_EXCL) : O_RDONLY, 0777);
}

int FileSystemHost::close(int fd) {
	return ::close(fd);
}

int FileSystemHost::read(int fd, void *buf, unsigned count) {
	return ::read(fd, buf, count);
}

int FileSystemHost::write(int fd, const void *buf, unsigned count) {
	return ::write(fd, buf, count);
}

int FileSystemHost::symlink(const char *target, const char *linkpath) {
	return ::symlink(target, linkpath);
}

int FileSystemHost::mkdir(const char *pathname) {
	return ::mkdir(pathname, ACCESSPERMS);
}

FileSystemInterface::DirHandle * FileSystemHost::opendir(const char *name) {
	return (FileSystemInterface::DirHandle *) ::opendir(name);
}

FileSystemInterface::DirEntry FileSystemHost::readdir(FileSystemInterface::DirHandle * dirp) {
	FileSystemInterface::DirEntry r;
	struct dirent * data = ::readdir(reinterpret_cast<DIR*>(dirp));
	if (data != nullptr) {
		r.inode_number = (unsigned long) data->d_ino;
		r.name = data->d_name;
		switch (data->d_type) {
			case DT_DIR: r.type = FileSystemInterface::DIRECTORY; break;
			case DT_LNK: r.type = FileSystemInterface::SYMLINK; break;
			case DT_REG: r.type = FileSystemInterface::FILE; break;
			default: r.type = FileSystemInterface::OTHER;
		}
	}
	return r;
}

void FileSystemHost::rewinddir(FileSystemInterface::DirHandle * dirp) {
	::rewinddir(reinterpret_cast<DIR*>(dirp));
}

int FileSystemHost::closedir(FileSystemInterface::DirHandle * dirp) {
	return ::closedir(reinterpret_cast<DIR*>(dirp));
}

FileSystemInterface::FileStats FileSystemHost::stat(const char *path, bool followSymlink) {
	FileSystemInterface::FileStats r;
	struct stat statbuf;
	errno = 0;
	int status = followSymlink ? ::stat(path, &statbuf) : ::lstat(path, &statbuf);
	r.error = errno;
	if (status == 0) {
		r.inode_number = statbuf.st_ino;
		r.hardlinks = statbuf.st_nlink;
		switch (statbuf.st_mode & S_IFMT) {
			case S_IFDIR: r.type = FileSystemInterface::DIRECTORY; break;
			case S_IFLNK: r.type = FileSystemInterface::SYMLINK; break;
			case S_IFREG: r.type = FileSystemInterface::FILE; break;
			default: r.type = FileSystemInterface::OTHER;
		}
		r.file_size = statbuf.st_size;
		r.block_size = statbuf.st_blksize;
		r.blocks = statbuf.st_blocks;
	}
	return r;
}

int FileSystemHost::getcwd(char *buf, unsigned bufsiz) {
	return ::getcwd(buf, bufsiz) == nullptr ? -1 : 0;
}

void FileSystemHost::sync() {
	::sync();
}

#pragma once

#include "fs/tool/fs_interface.h"

class FileSystemImage : public FileSystemInterface {
 public:
	FileSystemImage() {}

	int mount(char * buf, unsigned size);
	int umount();

	int chdir(const char *path);
	int readlink(const char *pathname, char *buf, unsigned bufsiz);

	int open(const char *pathname, bool write);
	int close(int fd);
	int read(int fd, void *buf, unsigned count);
	int write(int fd, const void *buf, unsigned count);

	int link(const char *oldpath, const char *newpath);
	int symlink(const char *target, const char *linkpath);
	int unlink(const char *pathname);

	int rename(const char *oldpath, const char *newpath);
	int mkdir(const char *pathname);
	int rmdir(const char *pathname);

	int truncate(const char *path);

	FileSystemInterface::DirHandle * opendir(const char *name);
	FileSystemInterface::DirEntry readdir(FileSystemInterface::DirHandle * dirp);
	void rewinddir(FileSystemInterface::DirHandle * dirp);
	int closedir(FileSystemInterface::DirHandle * dirp);

	FileSystemInterface::FileStats stat(const char *path, bool followSymlink = true);
	int getcwd(char *buf, unsigned bufsiz);

	void sync();
};

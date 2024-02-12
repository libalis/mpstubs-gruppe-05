#pragma once

#include "fs/tool/fs_interface.h"

class FileSystemHost : public FileSystemInterface {
 public:
	FileSystemHost() {}

	int chdir(const char *path);
	int readlink(const char *pathname, char *buf, unsigned bufsiz);

	int open(const char *pathname, bool write);
	int close(int fd);
	int read(int fd, void *buf, unsigned count);
	int write(int fd, const void *buf, unsigned count);

	int symlink(const char *target, const char *linkpath);

	int mkdir(const char *pathname);

	FileSystemInterface::DirHandle * opendir(const char *name);
	FileSystemInterface::DirEntry readdir(FileSystemInterface::DirHandle * dirp);
	void rewinddir(FileSystemInterface::DirHandle * dirp);
	int closedir(FileSystemInterface::DirHandle * dirp);

	FileSystemInterface::FileStats stat(const char *path, bool followSymlink = true);
	int getcwd(char *buf, unsigned bufsiz);

	void sync();
};

#pragma once

#define ENOTIMPLEMENTED 256

/* \brief Abstraktion aller interner Datenstrukturen
*/

class FileSystemInterface {
 public:
	enum FileType {
		FILE,
		DIRECTORY,
		SYMLINK,
		OTHER
	};

	struct FileStats {
		unsigned long inode_number;
		unsigned hardlinks;
		enum FileType type;
		unsigned file_size;
		unsigned block_size;
		unsigned blocks;
		int error;

		bool valid() const {
			return error == 0;
		}
	};

	struct DirEntry {
		unsigned long inode_number;
		enum FileType type;
		char * name;

		DirEntry() : inode_number(0), type(OTHER), name(nullptr) {}
		bool valid() const {
			return inode_number != 0 && name != nullptr;
		}
	};

	typedef void DirHandle;

	FileSystemInterface() {}
	virtual ~FileSystemInterface() {}

	virtual int chdir(const char *path) = 0;
	virtual int readlink(const char *pathname, char *buf, unsigned bufsiz) = 0;

	virtual int open(const char *pathname, bool write = false) = 0;
	virtual int close(int fd) = 0;
	virtual int read(int fd, void *buf, unsigned count) = 0;
	virtual int write(int fd, const void *buf, unsigned count) = 0;

	virtual int link(__attribute__((unused)) const char *oldpath, __attribute__((unused)) const char *newpath) {
		return -ENOTIMPLEMENTED;
	}

	virtual int symlink(const char *target, const char *linkpath) = 0;

	virtual int unlink(__attribute__((unused)) const char *pathname) {
		return -ENOTIMPLEMENTED;
	}

	virtual int rename(__attribute__((unused)) const char *oldpath, __attribute__((unused)) const char *newpath) {
		return -ENOTIMPLEMENTED;
	}

	virtual int mkdir(const char *pathname) = 0;
	virtual int rmdir(__attribute__((unused)) const char *pathname) {
		return -ENOTIMPLEMENTED;
	}

	virtual int truncate(__attribute__((unused)) const char *path) {
		return -ENOTIMPLEMENTED;
	}

	virtual DirHandle * opendir(const char *name) = 0;
	virtual DirEntry readdir(DirHandle * dirp) = 0;
	virtual void rewinddir(DirHandle * dirp) = 0;
	virtual int closedir(DirHandle * dirp) = 0;

	virtual struct FileStats stat(const char *path, bool followSymlink = true) = 0;
	virtual int getcwd(char *buf, unsigned bufsiz) = 0;

	virtual void sync() = 0;
};

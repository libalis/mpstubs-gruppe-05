#include "fs/tool/fs_image.h"

#include "fs/vfs.h"
#include "fs/errno.h"
#include "utils/string.h"

class Bufferdisk : public BlockDevice {
 private:
	char *buf;
	unsigned size;

 public:
	Bufferdisk() : buf(nullptr), size(0) {}
	~Bufferdisk() {}

	void load(char * buffer, unsigned buffersize) {
		buf = buffer;
		size = buffersize;
	}

	void unload() {
		buf = nullptr;
		size = 0;
	}

	Block fix(uint64_t block_number) {
		Block block(this, block_number);
		if ((block_number << blocksize_bits) >> blocksize_bits != block_number || (block_number << blocksize_bits) > size) {
			// FS bug
			block.data = nullptr;
			block.flags = -ENOSPC;
			return block;
		}
		block.data = buf + block_number * blocksize;
		return block;
	}

	void unfix(Block *block) {
		block->data = nullptr;
	}

	int sync() {
		return 0;
	}

	int sync(__attribute__((unused)) Block *block){
		return 0;
	}
};

static Bufferdisk bufferdisk;

int FileSystemImage::mount(char * buf, unsigned size) {
	bufferdisk.load(buf, size);
	return VFS::mount("minix", &bufferdisk, "");
}

int FileSystemImage::umount() {
	int i = VFS::umount();
	bufferdisk.unload();
	return i;
}

int FileSystemImage::chdir(const char *path) {
	return VFS::chdir(path);
}

int FileSystemImage::readlink(const char *pathname, char *buf, unsigned bufsiz) {
	return VFS::readlink(pathname, buf, bufsiz);
}

int FileSystemImage::open(const char *pathname, bool write) {
	return VFS::open(pathname, write ? (O_CREAT | O_WRONLY | O_EXCL) : O_RDONLY);
}

int FileSystemImage::close(int fd) {
	return VFS::close(fd);
}

int FileSystemImage::read(int fd, void *buf, unsigned count) {
	return VFS::read(fd, buf, count);
}

int FileSystemImage::write(int fd, const void *buf, unsigned count) {
	return VFS::write(fd, buf, count);
}

int FileSystemImage::link(const char *oldpath, const char *newpath) {
	return VFS::link(oldpath, newpath);
}

int FileSystemImage::symlink(const char *target, const char *linkpath) {
	return VFS::symlink(target, linkpath);
}

int FileSystemImage::unlink(const char *pathname) {
	return VFS::unlink(pathname);
}

int FileSystemImage::rename(const char *oldpath, const char *newpath) {
	return VFS::rename(oldpath, newpath);
}

int FileSystemImage::mkdir(const char *pathname) {
	return VFS::mkdir(pathname);
}

int FileSystemImage::rmdir(const char *pathname) {
	return VFS::rmdir(pathname);
}

int FileSystemImage::truncate(const char *path) {
	return VFS::truncate(path, 0);
}

FileSystemInterface::DirHandle * FileSystemImage::opendir(const char *name) {
	return (FileSystemInterface::DirHandle *) VFS::opendir(name);
}

FileSystemInterface::DirEntry FileSystemImage::readdir(FileSystemInterface::DirHandle * dirp) {
	FileSystemInterface::DirEntry r;
	Dirent * data = VFS::readdir(reinterpret_cast<DIR*>(dirp));
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

void FileSystemImage::rewinddir(FileSystemInterface::DirHandle * dirp) {
	VFS::rewinddir(reinterpret_cast<DIR*>(dirp));
}

int FileSystemImage::closedir(FileSystemInterface::DirHandle * dirp) {
	return VFS::closedir(reinterpret_cast<DIR*>(dirp));
}

FileSystemInterface::FileStats FileSystemImage::stat(const char *path, bool followSymlink) {
	FileSystemInterface::FileStats r;
	struct stat statbuf;
	r.error = (-1) * (followSymlink ? VFS::stat(path, &statbuf) : VFS::lstat(path, &statbuf));
	if (r.error == 0) {
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

int FileSystemImage::getcwd(char *buf, unsigned bufsiz) {
	if (bufsiz < 10)
		return -ENOBUFS;
	struct stat statbuf;
	int status = VFS::lstat(".", &statbuf);
	if (status == 0) {
		ino_t current_inode = statbuf.st_ino;
		status = VFS::lstat("/", &statbuf);
		if (status == 0) {
			ino_t root_inode = statbuf.st_ino;
			char up[bufsiz];
			size_t up_pos = 0;
			char down[bufsiz];
			down[0]='\0';
			down[1]='/';
			size_t down_pos = 2;
			while (current_inode != root_inode) {
				if (up_pos > bufsiz - 5) {
					status = -ENOBUFS;
					break;
				}
				up[up_pos++] = '.';
				up[up_pos++] = '.';
				up[up_pos++] = '/';
				up[up_pos] = '\0';
				DIR * dh = VFS::opendir(up);
				if (dh != nullptr) {
					ino_t next_inode = root_inode;
					while (1) {
						struct Dirent * de = VFS::readdir(dh);
						if (de == nullptr) {
							break;
						} else if (strcmp(de->d_name, ".") == 0) {
							next_inode = de->d_ino;
						} else if (strcmp(de->d_name, "..") == 0) {
							continue;
						} else if (de->d_ino == current_inode) {
							for (int i = static_cast<int>(strlen(de->d_name)); i >= 0; i--) {
								if (down_pos > sizeof(down) / sizeof(char) - 3) {
									return -ENOBUFS;
								} else {
									down[down_pos++] = de->d_name[i];
								}
							}
							down[down_pos++] = '/';
						}
					}
					if ((status = VFS::closedir(dh)) != 0) {
						break;
					}
					current_inode = next_inode;
				} else {
					break;
				}
			}
			if (current_inode == root_inode) {
				for (size_t i = 0; i < down_pos; i++) {
					buf[i] = down[down_pos - i - 1];
				}
			}
		}
	}
	return status;
}

void FileSystemImage::sync() {
	VFS::sync();
}

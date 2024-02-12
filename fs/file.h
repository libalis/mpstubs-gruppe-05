#pragma once

#include "types.h"
#include "fs/inode.h"

class File {
 public:
	int fd;
	int accmode;  // should be either O_RDONLY, O_WRONLY or O_RDWR
	Inode *inode;
	off_t pos;
	File *fd_table_next;
	// void *fs_data; private data for the owning filesystems (Minix doesn't need this)

	File(Inode *inode, int accmode) {
		fd = -1;
		this->accmode = accmode;
		this->inode = inode;
		pos = 0;
		fd_table_next = nullptr;
	}

	~File() {
		inode->put();
	}
};

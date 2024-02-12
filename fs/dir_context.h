#pragma once

#include "types.h"
#include "fs/definitions.h"

class Dir_Context {
 public:
	off_t pos;  // should be updated by the filesystem while iterating over a directory
	int error;

	explicit Dir_Context(off_t pos) : pos(pos), error(0) {}

	// Called by the filesystem for each entry while iterating over a directory.
	// Returns false to indicate that the filesystem should stop the iteration.
	virtual bool dir_emit(const char *name, size_t name_len, ino_t ino, unsigned char type) = 0;
};

struct Dirent;

class Readdir_Context : public Dir_Context {
 public:
	char *buf;
	size_t buf_used;  // in bytes
	size_t buf_size;  // in bytes

	Readdir_Context(off_t pos, Dirent *buf, size_t buf_size) : Dir_Context(pos), buf(reinterpret_cast<char*>(buf)),
	                                                           buf_used(0), buf_size(buf_size) {}

	bool dir_emit(const char *name, size_t name_len, ino_t ino, unsigned char type);
};

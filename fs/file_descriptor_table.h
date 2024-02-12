#pragma once

#include "fs/file.h"

class FD_Table {
 private:
	static const int max_files = 1024;
	static const size_t kBitmapSize = (max_files + 7) / 8;
	char bitmap[kBitmapSize];
	static const int fd_table_size = 8;
	File *file_descriptor_table[fd_table_size];

 public:
	FD_Table() {
		// @Cleanup
		for (unsigned int i = 0; i < kBitmapSize; i++) {
			bitmap[i] = 0;
		}
		for (int i = 0; i < fd_table_size; i++) {
			file_descriptor_table[i] = nullptr;
		}
	}

	// Inserts the given file into the table an sets its file descriptor field.
	// Returns false if there is no free space in the table.
	bool insert_file(File *file);

	// Removes and returns the 'File' associated with the file descriptor 'fd'.
	File *remove_file(int fd);

	// Returns the 'File' associated with the file descriptor 'fd'.
	File *get_file(int fd);
};

#pragma once

#include <string>
#include <vector>
#include <functional>

#include "fs_interface.h"

bool imageload(std::string path);
bool imagesync();
bool imageclose();
std::string imagename();
bool checkError(FileSystemInterface * const fs, int e);
bool printError(int e);
std::string getcwd(FileSystemInterface * const fs);
std::string readlink(FileSystemInterface * const fs, std::string symlink);
std::string join(const std::vector<std::string>& vec, const char* delim);
std::string filesize(unsigned long size, bool showBytes = false);
std::string basename(std::string const& path);
std::string dirname(std::string const& path);
bool recursive(FileSystemInterface * const fs, const std::string & path,
               const std::function<bool(const std::string &, const FileSystemInterface::FileType &)> callback);
bool list(FileSystemInterface * const fs, const std::string & path, bool recursive = false);
bool stat(FileSystemInterface * const fs, const std::string & path);
bool cat(FileSystemInterface * const fs, const std::string & path);
bool transfer(FileSystemInterface * const source, FileSystemInterface * const target,
              const std::string & path_source, std::string path_target, bool targetAsDir = true);

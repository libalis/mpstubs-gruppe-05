#include "fs/tool/util.h"

#include <algorithm>
#include <utility>
#include <iterator>
#include <iostream>
#include <sstream>

#include <cstring>
#include <cerrno>

#include "fs/tool/fs_image.h"
#include "fs/tool/fs_host.h"

#include "fs/tool/imagefile.h"

extern FileSystemImage image;
extern FileSystemHost host;

static ImageFile file;

#define count(X) (sizeof(X)/sizeof(X[0]))
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static std::string combinepath(const std::string & dir, const std::string & name) {
	std::stringstream path;
	path << dir;
	if (dir.back() != '/')
		path << '/';
	path << name;
	return path.str();
}

bool imageload(std::string path) {
	std::string name = basename(path);
	if (!file.open(path, name)) {
		std::cerr << "Abbruch, da das Abbild '" << name << "' nicht geladen werden konnte!" << std::endl;
	} else {
		int err = image.mount(file.buffer, file.size);
		if (err != 0)
			std::cerr << "Einhängen des Abbilds '" << name << "' fehlgeschlagen: " << strerror(-err) << std::endl;
		else
			return true;
	}
	return false;
}

bool imagesync() {
	std::cout << "Synchronisiere Abbild '" << file.name << "'..." << std::endl;
	image.sync();
	if (file.sync()) {
		return true;
	} else {
		std::cerr << "Synchronisation fehlgeschlagen, konnte nicht auf das Abbild zurückschreiben!" << std::endl;
		return false;
	}
}

bool imageclose() {
	image.sync();
	if (!file.sync() || !file.close()) {
		std::cerr << "Sichern des Abbilds fehlgeschlagen" << std::endl;
		return false;
	} else {
		return true;
	}
}

std::string imagename() {
	return file.name;
}

std::string basename(std::string const& path ) {
	return std::string(find_if(path.rbegin(), path.rend(), [](const char & c) { return c == '/'; }).base(), path.end());
}

std::string dirname(std::string const& path) {
	std::string dir;
	size_t i = path.rfind('/', path.length());
	if (i != std::string::npos)
		dir = path.substr(0, i);
	return dir;
}

std::string join(const std::vector<std::string>& vec, const char* delim) {
	std::stringstream res;
	copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(res, delim));
	return res.str();
}

bool checkError(FileSystemInterface * const fs, int e) {
	if (fs == &host) {
		if (e != 0) {
			e = errno;
			errno = 0;
		}
	} else if (fs == &image) {
		e *= -1;
	} else {
		std::cerr << "Ungültiger Dateisystemzeiger" << std::endl;
		return false;
	}
	return printError(e);
}

bool printError(int e) {
	if (e != 0) {
		std::cerr << "\e[31;1mFehler:\e[0;31m " << strerror(e) << "\e[0m" << std::endl;
		return false;
	} else {
		return true;
	}
}

std::string getcwd(FileSystemInterface * const fs) {
	std::string path;
	char buf[PATH_MAX + 1];
	if (checkError(fs, fs->getcwd(reinterpret_cast<char *>(::memset(reinterpret_cast<void *>(buf),
	                                                                '\0', count(buf))), count(buf) -1))) {
		path = buf;
	}
	return path;
}

std::string readlink(FileSystemInterface * const fs, std::string symlink) {
	std::string path;
	char buf[PATH_MAX];
	int r = fs->readlink(symlink.c_str(), buf, count(buf));
	if (r >= 0)
		path = std::string(buf, r);
	else
		checkError(fs, r);
	return path;
}

static bool recursive(FileSystemInterface * const fs, const std::string & path,
                      FileSystemInterface::FileType type,
                      const std::function<bool(const std::string &, const FileSystemInterface::FileType &)> callback) {
	bool status = true;
	if (type == FileSystemInterface::DIRECTORY) {
		FileSystemInterface::DirHandle * dh = fs->opendir(path.c_str());
		if (dh == nullptr) {
			status = false;
		} else {
			while (1) {
				FileSystemInterface::DirEntry de = fs->readdir(dh);
				if (de.valid()) {
					if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
						recursive(fs, combinepath(path, std::string(de.name)), de.type, callback);
					}
				} else {
					break;
				}
			}
			if (!checkError(fs, fs->closedir(dh)))
				status = false;
		}
	} else if (type == FileSystemInterface::OTHER) {
		std::cerr << "Ungültiger Typ von '" << path << "'!" << std::endl;
		return false;
	}
	return callback(path, type) && status;
}

bool recursive(FileSystemInterface * const fs, const std::string & path,
               const std::function<bool(const std::string &, const FileSystemInterface::FileType &)> callback) {
	FileSystemInterface::FileStats stats = fs->stat(path.c_str(), false);
	if (!stats.valid()) {
		std::cerr << "Ungültiger Pfad '" << path << "' in Rekursion: " << strerror(stats.error) << std::endl;
		return false;
	}
	return recursive(fs, path, stats.type, callback);
}

std::string filesize(unsigned long size, bool showBytes) {
	char buf[20];
	int i = 0;
	const char* units[] = {"", "K", "M", "G", "T", "P", "E", "Z", "Y"};
	double s = size * 1.0;
	for (; s > 1024; s /= 1024, i++) {}
	if (i > 0) {
		if (showBytes) {
			if (snprintf(buf, count(buf), "%.1f%s (%lu)", s, units[i], size) > 0) {
				return std::string(buf);
			}
		} else if (snprintf(buf, count(buf), "%.1f%s", s, units[i]) > 0) {
			return std::string(buf);
		}
	} else if (snprintf(buf, count(buf), "%.1lu", size) > 0) {
		return std::string(buf);
	}
	return std::string();
}

static bool listFile(FileSystemInterface * const fs, const std::string & path,
                     const std::string & name, unsigned recursive) {
	FileSystemInterface::FileStats stats = fs->stat(path.c_str(), false);
	if (!stats.valid()) {
		std::cerr << "Ungültiger Pfad '" << path << "' für Datei '" << name << "': " << strerror(stats.error) << std::endl;
		return false;
	} else {
		std::string t = "";
		std::string realpath;
		switch (stats.type) {
			case FileSystemInterface::FILE:
				t = " ";
				break;
			case FileSystemInterface::DIRECTORY:
				t = "D\e[1m";
				break;
			case FileSystemInterface::SYMLINK:
				t = "S\e[3m";
				if (!(realpath = readlink(fs, path)).empty())
					realpath.insert(0, " -> ");
				break;
			default:
				t = "?\e[31m";
		}
		printf("%10lu %8s %s  %*c%s\e[0m%s\n", stats.inode_number,  filesize(stats.file_size).c_str(),
		       t.c_str(), recursive * 2, ' ', name.c_str(), realpath.c_str());
		return true;
	}
}

static bool listDir(FileSystemInterface * const fs, const std::string & path, unsigned recursive) {
	FileSystemInterface::DirHandle * dh = fs->opendir(path.c_str());
	bool status = true;
	if (dh == nullptr) {
		status = false;
	} else {
		std::vector<std::pair<std::string, FileSystemInterface::FileType>> list;
		while (1) {
			FileSystemInterface::DirEntry de = fs->readdir(dh);
			if (de.valid()) {
				if (recursive == 0 || (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0)) {
					list.push_back(std::make_pair(std::string(de.name), de.type));
				}
			} else {
				break;
			}
		}
		if (!checkError(fs, fs->closedir(dh)))
			status = false;

		sort(list.begin(), list.end());
		for (auto e : list) {
			std::string fullpath = combinepath(path, e.first);
			if (!listFile(fs, fullpath, e.first, recursive))
				status = false;
			if (e.second == FileSystemInterface::DIRECTORY && recursive > 0 && e.first != "." && e.first != "..")
				if (!listDir(fs, fullpath, recursive + 1))
					status = false;
		}
	}
	return status;
}

bool list(FileSystemInterface * const fs, const std::string & path, bool recursive) {
	FileSystemInterface::FileStats stats = fs->stat(path.c_str(), true);
	bool r = false;
	if (!stats.valid()) {
		std::cerr << "Ungültiger Pfad '" << path << "': " << strerror(stats.error) << std::endl;
	} else {
		if (stats.type == FileSystemInterface::DIRECTORY) {
			r = listDir(fs, path, recursive ? 1 : 0);
		} else {
			r = listFile(fs, path, basename(path), -1);
		}
		std::cout << std::endl;
	}
	return r;
}

bool stat(FileSystemInterface * const fs, const std::string & path) {
	FileSystemInterface::FileStats stats = fs->stat(path.c_str(), false);
	if (!stats.valid()) {
		std::cerr << "Ungültiger Pfad '" << path << "': " << strerror(stats.error) << std::endl;
		return false;
	} else {
		switch (stats.type) {
			case FileSystemInterface::FILE:
				std::cout << "Datei: " << basename(path) << "\e[0m" << std::endl;
				break;
			case FileSystemInterface::DIRECTORY:
				std::cout << "Verzeichnis: \e[1m" << basename(path) << "\e[0m" << std::endl;
				break;
			case FileSystemInterface::SYMLINK:
				std::cout << "Symlink: \e[3m" << basename(path) << "\e[0m" << std::endl;
				 {
					std::string t = readlink(fs, path);
					if (!t.empty())
						std::cout << "Zielpfad: " << t << std::endl;
				 }
				break;
			default:
				std::cout << "Unbekanntes Etwas: \e[31m"<< basename(path) << "\e[0m" << std::endl;
		}
		std::cout << "Größe: " << filesize(stats.file_size, true) << std::endl;
		std::cout << "Blöcke: " << stats.blocks << " [bei " << filesize(stats.block_size) << " Blockgröße]" << std::endl;
		std::cout << "Indexknotennummer: " << stats.inode_number
		          << " [Harte Verweise: " << stats.hardlinks << "]" << std::endl << std::endl;
		return true;
	}
}

bool cat(FileSystemInterface * const fs, const std::string & path) {
	int fd = fs->open(path.c_str(), false);
	if (fd < 0) {
		return checkError(fs, fd);
	} else {
		bool status = true;
		char buf[1024];
		int size;
		while ((size = fs->read(fd, buf, count(buf))) > 0) {
			std::cout.write(buf, size);
		}
		if (size < 0)
			status = checkError(fs, size);
		return checkError(fs, fs->close(fd)) && status;
	}
}

bool transfer(FileSystemInterface * const source, FileSystemInterface * const target,
              const std::string & path_source, std::string path_target, bool targetAsDir) {
	bool status = true;
	FileSystemInterface::FileStats stats_source = source->stat(path_source.c_str(), true);
	if (!stats_source.valid()) {
		std::cerr << "Ungültiger Quellpfad '" << path_source << "': " << strerror(stats_source.error) << std::endl;
		return false;
	} else if (stats_source.type == FileSystemInterface::DIRECTORY) {
		if (path_target.empty())
			path_target = std::string(".");
		FileSystemInterface::DirHandle * dh = source->opendir(path_source.c_str());
		if (dh == nullptr) {
			std::cerr << "Quellverzeichnis '" << path_source << "' kann nicht gelesen werden!" << std::endl;
			status = false;
		} else {
			while (1) {
				FileSystemInterface::DirEntry de = source->readdir(dh);
				if (!de.valid()) {
					break;
				} else if (::strcmp(de.name, ".") != 0 && ::strcmp(de.name, "..") != 0) {
					std::string tmp_target = combinepath(path_target, std::string(de.name));
					FileSystemInterface::FileStats stats_target = target->stat(tmp_target.c_str(), false);
					bool createDir = de.type == FileSystemInterface::DIRECTORY;
					if (stats_target.valid() && createDir) {
						if (stats_target.type == FileSystemInterface::DIRECTORY) {
							createDir = false;
						} else if (!checkError(target, target->unlink(tmp_target.c_str()))) {
							std::cerr << "Kann '" << path_target << "' nicht löschen..." << std::endl;
							status = false;
							continue;
						}
					}
					if (createDir && !checkError(target, target->mkdir(tmp_target.c_str()))) {
						std::cerr << "Kann Zielverzeichnis '" << path_target << "' nicht erstellen!" << std::endl;
						status = false;
					} else if (!transfer(source, target, combinepath(path_source, std::string(de.name)), tmp_target, false)) {
						status = false;
					}
				}
			}
			if (!checkError(source, source->closedir(dh))) {
				status = false;
			}
		}
	} else {
		if (path_target.empty())
			path_target = basename(path_source);
		FileSystemInterface::FileStats stats_target = target->stat(path_target.c_str(), false);
		if (targetAsDir && stats_target.valid() && stats_target.type == FileSystemInterface::DIRECTORY) {
			path_target = combinepath(path_target, basename(path_source));
			stats_target = target->stat(path_target.c_str(), false);
		}
		if (stats_target.valid()) {
			if (stats_target.type == FileSystemInterface::DIRECTORY) {
				std::cerr << "Kann Quelldatei '" << path_source
				          << "' nicht kopieren - es existiert bereits ein Ordner im Ziel mit den Namen '"
						  << path_target << "' auf den Ziel!" << std::endl;
				return false;
			} else if (checkError(target, target->unlink(path_target.c_str()))) {
				std::cout << "Lösche vorhandene Datei '" << path_target << "'..." << std::endl;
			} else {
				std::cerr << "Vorhandene Zieldatei '" << path_target << "' kann nicht gelöscht werden!" << std::endl;
				return false;
			}
		}
		int fd_source = source->open(path_source.c_str(), false);
		if (fd_source < 0) {
			std::cerr << "Kann Quelldatei '" << path_source << "' nicht öffnen!" << std::endl;
			return checkError(source, fd_source);
		} else {
			int fd_target = target->open(path_target.c_str(), true);
			if (fd_target < 0) {
				std::cerr << "Kann Zieldatei '" << path_source << "' nicht erstellen/öffnen!" << std::endl;
				checkError(target, fd_target);
				status = false;
			} else {
				std::cout << "Kopiere '" << path_source << "'..." << std::endl;
				char buf[4096];
				int size_in, size_out;
				while ((size_in = source->read(fd_source, buf, count(buf))) > 0) {
					if ((size_out = target->write(fd_target, buf, size_in)) < 0 && !checkError(target, size_out)) {
						status = false;
						break;
					} else if (size_out != size_in) {
						std::cerr << "Es konnten nur " << size_out << " der " << size_in
						          << " Bytes im Puffer in die Datei '" << path_target
								  << "' geschrieben werden!" << std::endl;
						status = false;
						break;
					}
				}
				if (size_in < 0) {
					status = checkError(source, size_in);
				}
				if (!checkError(target, target->close(fd_target))) {
					status = false;
				}
			}
			if (!checkError(source, source->close(fd_source))) {
				status = false;
			}
		}
	}
	return status;
}

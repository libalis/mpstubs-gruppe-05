#include "fs/tool/parameter.h"

#include <sstream>
#include <iostream>
#include <cstring>

#include "fs/tool/util.h"
#include "fs/tool/fs_image.h"
#include "fs/tool/fs_host.h"

extern FileSystemImage image;
extern FileSystemHost host;

bool Parameter::checkType(enum FileSystemInterface::FileType ft) const {
	if (type == TEXT) {
		return true;
	} else {
		switch (ft) {
			case FileSystemInterface::DIRECTORY: return type == DIRECTORY || type == PATH;
			case FileSystemInterface::SYMLINK: return type == SYMLINK || type == PATH;
			case FileSystemInterface::FILE: return type == FILE || type == PATH;
			default: return false;
		}
	}
}

bool Parameter::check(const std::string & arg) const {
	if (arg.empty()) {
		return optional || type == TEXT;
	} else {
		FileSystemInterface::FileStats stats = local ? host.stat(arg.c_str(), false) : image.stat(arg.c_str(), false);
		if (stats.valid() && status == MUST_NOT_EXIST) {
			std::cerr << "Fehler: " << getName() << " '" << arg << "' existiert bereits!" << std::endl;
			return false;
		} else if (!stats.valid()) {
			if (status == MUST_EXIST) {
				std::cerr << "Fehler: " << getName() << " '" << arg << "' ist nicht verfügbar ("
				          << strerror(stats.error) << ")!" << std::endl;
				return false;
			} else {
				return true;
			}
		} else if (!checkType(stats.type)) {
			std::cerr << "Fehler: '" << arg << "' ist nicht vom Typ " << getName() << "!" << std::endl;
			return false;
		} else {
			return true;
		}
	}
}

void Parameter::complete(const std::string & in, const std::string & part,
                         std::vector<std::string>& completions) const {
	if (status != MUST_NOT_EXIST) {
		FileSystemInterface * fs;
		if (local) {
			fs = &host;
		} else {
			fs = &image;
		}
		std::string dir = dirname(part);
		std::string base = basename(part);
		FileSystemInterface::DirHandle * dh = fs->opendir(dir.empty() ? "." : dir.c_str());
		if (dh != nullptr) {
			while (1) {
				FileSystemInterface::DirEntry de = fs->readdir(dh);
				if (de.valid()) {
					if (de.name != nullptr && checkType(de.type)) {
						std::string name(de.name);
						if (name.size() >= base.size() && name.compare(0, base.size(), base) == 0) {
							std::stringstream ss;
							ss << in;
							if (!dir.empty())
								ss << dir << "/";
							ss << name;
							if (de.type == FileSystemInterface::DIRECTORY)
								ss << "/";
							completions.push_back(ss.str());
						}
					}
				} else {
					break;
				}
			}
			fs->closedir(dh);
		}
	}
}

std::string Parameter::getName(void) const {
	if (name.empty()) {
		switch(type) {
			case Parameter::FILE:
				return "Datei";
			case Parameter::DIRECTORY:
				return "Verzeichnis";
			case Parameter::SYMLINK:
				return "Sym.Verknüpfung";
			case Parameter::PATH:
				return "Pfad";
			default:
				return "Text";
		}
	} else {
		return name;
	}
}

std::ostream& operator<<(std::ostream &stream, const Parameter &p) {
	if (p.optional)
		stream << '[';
	stream << "\e[" << (p.local ? 34 : 35) << 'm' << p.getName() << "\e[0m";
	if (p.optional)
		stream << ']';

	return stream;
}

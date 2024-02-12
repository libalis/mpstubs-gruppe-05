#pragma once

#include <string>
#include <vector>
#include <ostream>

#include "fs_interface.h"

struct Parameter {
	const bool local;  // ist der Pfad lokal (also auf dem Host)?
	const enum Type {
		TEXT,
		FILE,
		DIRECTORY,
		SYMLINK,
		PATH,
	} type;
	const enum Exist {
		MAY_EXIST,
		MUST_EXIST,
		MUST_NOT_EXIST
	} status;  // Status der Datei?
	const bool optional;  // ist der Parameter optional

	const std::string name;

	Parameter() : local(false), type(TEXT), status(MAY_EXIST), optional(false), name() {}
	Parameter(const bool && local, const enum Type && type, const enum Exist && status = MUST_EXIST,
	          const bool && optional = false, const std::string name = std::string()) :
	    local(local), type(type), status(status), optional(optional), name(name) {}

	bool check(const std::string & arg) const;

	void complete(const std::string & in, const std::string & part, std::vector<std::string>& completions) const;

	std::string getName(void) const;

 private:
	bool checkType(enum FileSystemInterface::FileType ft) const;
};

std::ostream& operator<<(std::ostream &stream, const Parameter &p);

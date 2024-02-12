#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <functional>

#include "parameter.h"

struct Command {
	typedef std::function<bool (const std::vector<std::string> &)> callback;

	const std::vector<std::string> name;  // Kommando und Alias(e)
	const std::vector<Parameter> args;  // Parameter
	const std::string description;  // Hilfe

	const callback action;  // Funktionalität

	Command(const std::vector<std::string> &&name, const std::vector<Parameter> &&args, const std::string &&description,
	        const callback action) : name(name), args(args), description(description), action(action) {}
	Command(const std::vector<std::string> &&name, const std::string &&description, const callback action) :
	    name(name), args(0), description(description), action(action) {}

	static bool execute(std::string action, const std::vector<std::string> & param);
	static bool execute(const std::vector<std::string> & parts);
	static bool execute(const std::string & line);

	static void complete(const char* input, std::vector<std::string>& completions);

	static void help();
};

std::ostream& operator<<(std::ostream &stream, const Command &c);

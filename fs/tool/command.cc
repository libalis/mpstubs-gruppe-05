#include "fs/tool/command.h"

#include <unistd.h>

#include <cstdlib>
#include <cstdio>

#include <algorithm>
#include <iterator>
#include <iostream>
#include <sstream>

#include "fs/tool/parameter.h"

#include "fs/tool/imagefile.h"
#include "fs/tool/fs_image.h"
#include "fs/tool/fs_host.h"
#include "fs/tool/util.h"

extern FileSystemImage image;
extern FileSystemHost host;

typedef const std::vector<std::string> & param;
Command::callback undefined = [](__attribute__((unused)) const std::vector<std::string> & p) {
	std::cerr << "Befehl kann nicht ausgeführt werden, da er noch nicht implementiert wurde!" << std::endl;
	return false;
};

Command commands[] = {
// Generische Kommandos
	Command(
		{ "help" , "h", "?" },
		"Zeige diese Hilfe an",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			Command::help();
			return true;
		}
	),

	Command(
		{ "quit" , "bye", "exit" },
		"Synchronisiere das Abbild und beende die Anwendung (für interaktive Konsole)",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			bool status = imageclose();
			exit(status ? EXIT_SUCCESS : EXIT_FAILURE);
			return status;
		}
	),

	Command(
		{ "abort" },
		"Beende die Anwendung ohne vorherige Synchronisation (für interaktive Konsole)",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			exit(EXIT_FAILURE);
			return false;
		}
	),

// Befehle zum Arbeiten auf dem Dateisystem des Abbilds
	Command(
		{ "cd" },
		{ Parameter(false, Parameter::DIRECTORY) },
		"Wechsel in das Verzeichnis",
		[](const std::vector<std::string> & p) {
			return checkError(&image, image.chdir(p.front().c_str()));
		}
	),

	Command(
		{ "cat", "show" },
		{ Parameter(false, Parameter::FILE) },
		"Zeige Inhalt der Datei",
		[](const std::vector<std::string> & p) {
			return cat(&image, p.front());
		}
	),

	Command(
		{ "delete", "rm" },
		{ Parameter(false, Parameter::PATH) },
		"Lösche die Datei oder Verzeichnis (rekursiv)",
		[](const std::vector<std::string> & p) {
			return recursive(&image, p.size() > 0 ? p.front() : ".",
			   [](const std::string & path, FileSystemInterface::FileType type) {
				if (type == FileSystemInterface::DIRECTORY) {
					return checkError(&image, image.rmdir(path.c_str()));
				} else {
					return checkError(&image, image.unlink(path.c_str()));
				}
			});
		}
	),

	Command(
		{ "dir" , "ls" },
		{ Parameter(false, Parameter::PATH, Parameter::MUST_EXIST, true) },
		"Zeige den Inhalt des aktuellen/angegebenen Verzeichnis an",
		[](const std::vector<std::string> & p) {
			return list(&image, p.size() > 0 ? p.front() : ".", false);
		}
	),

	Command(
		{ "tree" },
		{ Parameter(false, Parameter::DIRECTORY, Parameter::MUST_EXIST, true) },
		"Zeige den Verzeichnisbaum des aktuellen/angegebenen Verzeichnis rekursiv an",
		[](const std::vector<std::string> & p) {
			return list(&image, p.size() > 0 ? p.front() : ".", true);
		}
	),

	Command(
		{ "link" },
		{
			Parameter(false, Parameter::FILE, Parameter::MUST_EXIST, false, "Quelle"),
			Parameter(false, Parameter::FILE, Parameter::MUST_NOT_EXIST, true, "Ziel")
		},
		"Erstelle einen harten Verweis auf den Inhalt der angegebenen Datei",
		[](const std::vector<std::string> & p) {
			return checkError(&image, image.link(p.at(0).c_str(), p.size() > 1 ? p.at(1).c_str() : basename(p.at(0)).c_str()));
		}
	),

	Command(
		{ "mkdir" },
		{ Parameter(false, Parameter::DIRECTORY, Parameter::MUST_NOT_EXIST) },
		"Erstelle ein Verzeichnis",
		[](const std::vector<std::string> & p) {
			return checkError(&image, image.mkdir(p.front().c_str()));
		}
	),

	Command(
		{ "move", "mv", "rename" },
		{
			Parameter(false, Parameter::PATH, Parameter::MUST_EXIST, false, "Quelle"),
			Parameter(false, Parameter::PATH, Parameter::MAY_EXIST, false, "Ziel")
		},
		"Verschiebe eine Datei oder Ordner in das angegebenen Verzeichnis",
		[](const std::vector<std::string> & p) {
			return checkError(&image, image.rename(p.at(0).c_str(), p.at(1).c_str()));
		}
	),

	Command(
		{ "pwd" },
		"Gibt den Pfad zum aktuellen Verzeichnis aus",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			std::string cwd = getcwd(&image);
			std::cout << cwd << std::endl;
			return !cwd.empty();
		}
	),

	Command(
		{ "readlink" },
		{ Parameter(false, Parameter::PATH) },
		"Zeige das Ziel der symbolischen Verknüpfung",
		[](const std::vector<std::string> & p) {
			std::string path = readlink(&image, p.front());
			if (!path.empty()) {
				std::cout << path << std::endl;
				return true;
			} else {
				return false;
			}
		}
	),

	Command(
		{ "rmdir" },
		{ Parameter(false, Parameter::DIRECTORY, Parameter::MUST_EXIST) },
		"Entfernt ein (leeres) Verzeichnis",
		[](const std::vector<std::string> & p) {
			return checkError(&image, image.rmdir(p.front().c_str()));
		}
	),

	Command(
		{ "stat" },
		{ Parameter(false, Parameter::PATH, Parameter::MUST_EXIST) },
		"Zeige die Metainformationen der Datei",
		[](const std::vector<std::string> & p) {
			return stat(&image, p.front().c_str());
		}
	),

	Command(
		{ "symlink" },
		{
			Parameter(false, Parameter::PATH, Parameter::MUST_EXIST, false, "Quelle"),
			Parameter(false, Parameter::FILE, Parameter::MUST_NOT_EXIST, true, "Ziel")
		},
		"Erstelle eine symbolische Verknüpfung auf den Inhalt der angegebenen Datei",
		[](const std::vector<std::string> & p) {
			if (p.size() > 1) {
				return checkError(&image, image.symlink(p.at(0).c_str(), p.at(1).c_str()));
			} else {
				return checkError(&image, image.symlink(p.at(0).c_str(), basename(p.at(0)).c_str()));
			}
		}
	),

	Command(
		{ "trunc" },
		{ Parameter(false, Parameter::FILE) },
		"Lösche den Inhalt der Datei",
		[](const std::vector<std::string> & p) {
			return checkError(&image, image.truncate(p.front().c_str()));
		}
	),

// Befehle für das lokale (Wirts)Dateisystem
	Command(
		{ "lcat", "lshow" },
		{ Parameter(true, Parameter::FILE) },
		"Zeige Inhalt der lokalen Datei",
		[](const std::vector<std::string> & p) {
			return cat(&host, p.front());
		}
	),

	Command(
		{ "lcd" },
		{ Parameter(true, Parameter::DIRECTORY) },
		"Wechsel in das lokale Verzeichnis",
		[](const std::vector<std::string> & p) {
			return checkError(&host, image.chdir(p.front().c_str()));
		}
	),

	Command(
		{ "ldir" , "lls" },
		{ Parameter(true, Parameter::PATH, Parameter::MUST_EXIST, true) },
		"Zeige den Inhalt des aktuellen/angebenen lokalen Verzeichnisses an",
		[](const std::vector<std::string> & p) {
			return list(&host, p.size() > 0 ? p.front() : ".", false);
		}
	),

	Command(
		{ "ltree" },
		{ Parameter(true, Parameter::DIRECTORY, Parameter::MUST_EXIST, true) },
		"Zeige den Verzeichnisbaum des aktuellen/angebenen lokalen Verzeichnisses rekursiv an",
		[](const std::vector<std::string> & p) {
			return list(&host, p.size() > 0 ? p.front() : ".", true);
		}
	),

	Command(
		{ "lpwd" },
		"Gibt den Pfad zum aktuellen lokalen Verzeichnis aus",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			std::string cwd = getcwd(&host);
			std::cout << cwd << std::endl;
			return !cwd.empty();
		}
	),

	Command(
		{ "lstat" },
		{ Parameter(true, Parameter::PATH, Parameter::MUST_EXIST) },
		"Zeige die Metainformationen der lokalen Datei",
		[](const std::vector<std::string> & p) {
			return stat(&host, p.front().c_str());
		}
	),

// Befehle für die Übertragung
	Command(
		{ "put" },
		{
			Parameter(true, Parameter::PATH, Parameter::MUST_EXIST, false, "Quelle"),
			Parameter(false, Parameter::PATH, Parameter::MAY_EXIST, true, "Ziel")
		},
		"Übertrage die lokale Datei/Verzeichnis ins Abbild.",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			return transfer(&host, &image, p.at(0), p.size() > 1 ? p.at(1) : std::string());
		}
	),

	Command(
		{ "get" },
		{
			Parameter(false, Parameter::PATH, Parameter::MUST_EXIST, false, "Quelle"),
			Parameter(true, Parameter::PATH, Parameter::MAY_EXIST, true, "Ziel")
		},
		"Empfange die Datei/das Verzeichnis vom Abbild.",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			return transfer(&image, &host, p.at(0), p.size() > 1 ? p.at(1) : std::string());
		}
	),

	Command(
		{ "sync" },
		"Synchronisiere das Abbild (für interaktive Konsole)",
		[](__attribute__((unused)) const std::vector<std::string> & p) {
			return imagesync();
		}
	),
};

// Split String by non-escaped whitespace, based on https://stackoverflow.com/a/29384080
static std::vector<std::string> split(const std::string & in) {
	auto f = in.begin();
	auto l = in.end();
	std::vector<std::string> out;
	std::string accum;
	auto flush = [&] {
		if (!accum.empty()) {
			out.push_back(accum);
			accum.resize(0);
		}
	};

	while (f != l) {
		switch(*f) {
			case '\\':
				if (++f != l && *f == ' ') {
					accum += ' ';
					f++;
				} else {
					accum += '\\';
				}
				break;
			case ' ': case '\t': case '\r': case '\n':
				++f;
				flush();
				break;
			default:
				accum += *f++;
		}
	}
	flush();
	return out;
}

void Command::complete(const char* input, std::vector<std::string>& completions) {
	std::string prefix, last, in(input);
	std::vector<std::string> args = split(in);

	size_t params = args.size();
	// Space at the end indicates a new argument
	if (in.back() != ' ') {
		params--;
		last = args.back();
	}
	for (const Command & cmd : commands)
		for (const std::string & name : cmd.name)
			// Complete command
			if (params == 0 && name.size() >= in.size() && name.compare(0, in.size(), in) == 0) {
				completions.push_back(name);
			} else if (params > 0 && name.compare(args[0]) == 0 && cmd.args.size() >= params) {
				if (prefix.empty())
					prefix = in.substr(0, in.rfind(last));
				cmd.args[params - 1].complete(prefix, last, completions);
			}
	sort(completions.begin(), completions.end());
}

void Command::help() {
	std::cout << std::endl << "\e[1;4mVerfügbare Befehle:\e[0m" << std::endl << std::endl;
	for (Command c : commands)
		std::cout << c << std::endl;
}

bool Command::execute(std::string action, const std::vector<std::string> & param) {
	if (!action.empty()) {
		transform(action.begin(), action.end(), action.begin(), ::tolower);
		for (const Command & cmd : commands)
			for (const std::string & name : cmd.name)
				if (action == name) {
					if (param.size() > cmd.args.size()) {
						std::cerr << "Fehler: Zu viele Parameter angegeben!" << std::endl << "Verwendung: " << cmd << std::endl;
						return false;
					} else {
						for (size_t i = 0; i < cmd.args.size(); i++) {
							if (!cmd.args[i].check(param.size() > i ? param.at(i) : std::string())) {
								return false;
							}
						}
						return cmd.action(param);
					}
				}
		std::cerr << "Unbekannter Befehl '" << action << "'!" << std::endl;
	}
	return false;
}

bool Command::execute(const std::vector<std::string> & parts) {
	if (parts.size() == 0) {
		return true;
	} else {
		return execute(parts.front(), std::vector<std::string>(parts.begin() + 1, parts.end()));
	}
}

bool Command::execute(const std::string & line) {
	return line.empty() ? true : execute(split(line));
}

std::ostream& operator<<(std::ostream &stream, const Command &c) {
	stream << "\t\e[1m" << c.name[0] << "\e[0m";
	for (const Parameter &p : c.args)
		stream << " " << p;
	stream << std::endl << "\t\t" << c.description << std::endl;
	if (c.name.size() > 1) {
		stream << "\t\t\e[3m(Alias: " << c.name[1];
		for (unsigned i = 2; i < c.name.size(); i++)
			stream << ", " << c.name[i];
		stream << ")\e[0m " << std::endl;
	}
	return stream;
}

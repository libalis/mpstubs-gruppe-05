#include <cstring>
#include <string>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>

#include <fstream>

// Kompakte Readline-Alternative, siehe https://github.com/yhirose/cpp-linenoise
#include "linenoise.hpp"

#include "fs/tool/parameter.h"
#include "fs/tool/command.h"
#include "fs/tool/util.h"

#include "fs/tool/fs_image.h"
#include "fs/tool/fs_host.h"

static const char * history_path = ".fstool.history";

FileSystemImage image;
FileSystemHost host;

static void console(void) {
	std::cout << "\e[2mStarte interaktive Konsole..." << std::endl << std::endl;

	linenoise::SetMultiLine(true);
	linenoise::SetCompletionCallback(Command::complete);
	linenoise::LoadHistory(history_path);

	bool success = true;
	while (true) {
		// Generate prompt
		std::string line;

		std::stringstream ss;
		ss << "\e[1m" << imagename() << "\e[0m:\e[35m" << getcwd(&image) << "\e[0m" << (success ? "> " : "\e[31m>\e[0m ");

		auto quit = linenoise::Readline(ss.str().c_str(), line);
		if (quit) {
			break;
		}

		success = Command::execute(line);

		linenoise::AddHistory(line.c_str());
		linenoise::SaveHistory(history_path);
	}
}

int main(int argc, const char** argv) {
	std::cout << std::endl << "\e[1;4mWerkzeug für das Minix v3-Dateisystem in StuBS\e[0m" << std::endl << std::endl;
	if (argc < 2) {
		std::cout << "Verwendung:" << std::endl << "\t\e[3m" << argv[0] << " [Befehl(e)] Abbild\e[0m" << std::endl
		          << std::endl;
		std::cout << "Die Befehle können entweder in üblicher Parameterschreibweise gelistet werden: " << std::endl
		          << "\t\e[3m" << argv[0] << " -mkdir foo -put local foo minix.img\e[0m" << std::endl;
		std::cout << "Alternativ werden sie durch einen freistehenden Doppelpunkt getrennt:" << std::endl
		          << "\t\e[3m" << argv[0] << " mkdir foo : put local foo minix.img\e[0m" << std::endl;
		std::cout << "Eine implizite Synchronisation des Abbilds erfolgt automatisch bei erfolgreicher Abarbeitung"
		          << " aller genannten Befehle." << std::endl << std::endl;
		std::cout << "Sofern kein Befehl übergeben wird startet die interaktive Konsole." << std::endl;
		Command::help();
		return EXIT_SUCCESS;
	} else if (imageload(argv[argc-1])) {
		if (argc == 2) {
			console();
		} else {
			if (argv[1][0] == '-') {
				std::vector<std::string> parts;
				parts.push_back(&(argv[1][1]));
				for (int i = 2; i < argc; i++) {
					if (argv[i][0] == '-' || i == argc - 1) {
						std::cout << std::endl << "\e[2m" << join(parts, " ") << "\e[0m" << std::endl;
						if (!Command::execute(parts)) {
							std::cerr << "Abbruch, da Befehl '" << parts.front() << "' fehlschlug!" << std::endl;
							return EXIT_FAILURE;
						} else {
							parts.clear();
						}
						parts.push_back(&(argv[i][1]));
					} else {
						parts.push_back(argv[i]);
					}
				}
			} else {
				std::vector<std::string> parts;
				for (int i = 1; i < argc; i++) {
					if ((argv[i][0] == ':' && argv[i][1] == '\0') || i == argc - 1) {
						std::cout << std::endl << "\e[2m" << join(parts, " ") << "\e[0m" << std::endl;
						if (!Command::execute(parts)) {
							std::cerr << "Abbruch, da Befehl '" << parts.front() << "' fehlschlug!" << std::endl;
							return EXIT_FAILURE;
						} else {
							parts.clear();
						}
					} else {
						parts.push_back(argv[i]);
					}
				}
			}
		}
		if (imageclose())
			return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

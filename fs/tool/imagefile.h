#pragma once

#include <string>
#include <iostream>
#include <fstream>

class ImageFile {
	std::fstream file;

 public:
	size_t size;
	char *buffer;
	std::string name;

	~ImageFile() {
		if (file.is_open())
			close();
	}

	bool open(std::string path, std::string name) {
		this->name = name;
		if (file.is_open()) {
			std::cerr << "Fehler: Abbild '" << name << "' ist bereits geöffnet!" << std::endl;
		} else {
			std::cout << "Öffne Abbild '" << name << "'..." << std::endl;
			file.open(path, std::fstream::in | std::fstream::out | std::fstream::binary);
			if (file.is_open()) {
				// Dateigröße ermitteln
				file.seekg(0, file.end);
				size = file.tellg();
				file.seekg(0, file.beg);

				std::cout << "Lese " << size << " Bytes..." << std::endl;

				// allokiere Puffer
				buffer = new char[size];
				// Daten einlesen
				file.read(buffer, size);

				// Fehlerprüfung
				if (file)
					return file.good();
				else
					std::cerr << "Fehler: " << file.gcount() << " / " << size << " Bytes gelesen, Abbruch" << std::endl;
			} else {
				std::cerr << "Öffnen fehlgeschlagen" << std::endl;
			}
		}
		return false;
	}

	bool close() {
		if (file.is_open()) {
			file.close();
			delete[] buffer;
			return true;
		} else {
			std::cerr << "Abbild kann nicht geschlossen werden, da nicht geöffnet!" << std::endl;
			return false;
		}
	}

	bool sync() {
		if (file.is_open()) {
			file.seekp(0, file.beg);
			file.write(buffer, size);
			file.flush();
			return file.good();
		} else {
			std::cerr << "Abbild kann nicht synchronisiert werden, da nicht geöffnet!" << std::endl;
			return false;
		}
	}
};

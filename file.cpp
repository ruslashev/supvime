// file.cpp
// Files I/O operations - read, write etc

#include "file.hpp"

File::File(std::string fileToOpen)
{
	filename = fileToOpen;
	std::ifstream ifs(fileToOpen, std::ifstream::in);
	if (!ifs) {
		printf("No such file \"%s\"\n", fileToOpen.c_str());
		exit(1);
	}

	std::string lineBuf;
	getline(ifs, lineBuf);
	while (ifs.good()) {
		lines.push_back(lineBuf);
		getline(ifs, lineBuf);
	}
}


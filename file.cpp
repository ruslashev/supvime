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

	std::string lineBuf = "";
	while (getline(ifs, lineBuf)) {
		lines.push_back(lineBuf);
		// printf("%s\n", lineBuf.c_str());
	}

	ifs.close();
}


// file.cpp
// Files I/O operations - read, write etc

#include "file.hpp"

void File::Open(std::string fileToOpen)
{
	filename = fileToOpen;
	std::ifstream ifs(fileToOpen, std::ifstream::in);
	if (!ifs)
		throwf("No such file \"%s\"\n", fileToOpen.c_str());

	std::string lineBuf = "";
	while (getline(ifs, lineBuf)) {
		lines.push_back(lineBuf);
		// printf("%s\n", lineBuf.c_str());
	}

	ifs.close();
}


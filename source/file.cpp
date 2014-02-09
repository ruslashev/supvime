// file.cpp
// Files I/O operations - read, write etc

#include "file.hpp"
#include "errors.hpp"

void File::Open(std::string fileToOpen)
{
	filename = fileToOpen;
	std::ifstream ifs(fileToOpen, std::ifstream::in);
	if (!ifs)
		throwf("No such file \"%s\"", fileToOpen.c_str());

	std::string lineBuf = "";
	while (getline(ifs, lineBuf)) {
		lines.push_back(lineBuf);
	}

	ifs.close();
}

